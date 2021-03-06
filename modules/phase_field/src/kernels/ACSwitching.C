/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACSwitching.h"

template <>
InputParameters
validParams<ACSwitching>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription(
      "Kernel for Allen-Cahn equation that adds derivatives of switching functions * energies");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Fj_names", "List of free energies for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide h. Place in same order as Fj_names!");
  return params;
}

ACSwitching::ACSwitching(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _nvar(_coupled_moose_vars.size()),
    _etai_name(_var.name()),
    _Fj_names(getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _prop_dFjdarg(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_dhjdetai(_num_j),
    _prop_d2hjdetai2(_num_j),
    _prop_d2hjdetaidarg(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != _hj_names.size())
    mooseError("Need to pass in as many hj_names as Fj_names in ACSwitching ", name());

  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    // get phase free energy
    _prop_Fj[n] = &getMaterialPropertyByName<Real>(_Fj_names[n]);
    _prop_dFjdarg[n].resize(_nvar);

    // get switching derivatives wrt eta_i, the nonlinear variable
    _prop_dhjdetai[n] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name);
    _prop_d2hjdetai2[n] =
        &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, _etai_name);
    _prop_d2hjdetaidarg[n].resize(_nvar);

    for (unsigned int i = 0; i < _nvar; ++i)
    {
      MooseVariable * cvar = _coupled_moose_vars[i];
      // Get derivatives of all Fj wrt all coupled variables
      _prop_dFjdarg[n][i] = &getMaterialPropertyDerivative<Real>(_Fj_names[n], cvar->name());

      // Get second derivatives of all hj wrt eta_i and all coupled variables
      _prop_d2hjdetaidarg[n][i] =
          &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, cvar->name());
    }
  }
}

void
ACSwitching::initialSetup()
{
  ACBulk<Real>::initialSetup();

  for (unsigned int n = 0; n < _num_j; ++n)
  {
    validateNonlinearCoupling<Real>(_Fj_names[n]);
    validateNonlinearCoupling<Real>(_hj_names[n]);
  }
}

Real
ACSwitching::computeDFDOP(PFFunctionType type)
{
  Real sum = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];

      return sum;

    case Jacobian:
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_d2hjdetai2[n])[_qp] * (*_prop_Fj[n])[_qp];

      return _phi[_j][_qp] * sum;
  }

  mooseError("Invalid type passed in to ACSwitching::computeDFDOP");
}

Real
ACSwitching::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  // Then add dependence of ACSwitching on other variables
  Real sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_d2hjdetaidarg[n][cvar])[_qp] * (*_prop_Fj[n])[_qp] +
           (*_prop_dhjdetai[n])[_qp] * (*_prop_dFjdarg[n][cvar])[_qp];

  res += _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
