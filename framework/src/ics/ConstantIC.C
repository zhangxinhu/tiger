/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ConstantIC.h"
#include "libmesh/point.h"

template <>
InputParameters
validParams<ConstantIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("value", "The value to be set in IC");
  return params;
}

ConstantIC::ConstantIC(const InputParameters & parameters)
  : InitialCondition(parameters), _value(getParam<Real>("value"))
{
}

Real
ConstantIC::value(const Point & /*p*/)
{
  return _value;
}
