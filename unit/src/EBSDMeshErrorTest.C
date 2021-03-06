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

#include "EBSDMeshErrorTest.h"

TEST_F(EBSDMeshErrorTest, fileDoesNotExist)
{
  // generate input parameter set
  InputParameters params = validParams<EBSDMesh>();
  params.addPrivateParam("_moose_app", _app);
  params.set<std::string>("_object_name", "EBSD");

  // set filename
  params.set<FileName>("filename") = "FILEDOESNOTEXIST";

  // construct mesh object
  std::unique_ptr<EBSDMesh> mesh(new EBSDMesh(params));

  try
  {
    // trigger mesh building with invalid EBSD filename
    mesh->buildMesh();
    FAIL() << "buildMesh should have failed but didn't";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Can't open EBSD file: FILEDOESNOTEXIST") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST_F(EBSDMeshErrorTest, headerError)
{
  const unsigned int ntestcase = 4;

  const char * testcase[ntestcase][2] = {
      {"data/ebsd/ebsd3D_zerostep.txt", "Error reading header, EBSD data step size is zero."},
      {"data/ebsd/ebsd3D_zerosize.txt", "Error reading header, EBSD grid size is zero."},
      {"data/ebsd/ebsd3D_zerodim.txt", "Error reading header, EBSD data is zero dimensional."},
      {"data/ebsd/ebsd3D_norefine.txt",
       "EBSDMesh error. Requested uniform_refine levels not possible."},
  };

  for (unsigned int i = 0; i < ntestcase; ++i)
  {
    auto filename = testcase[i][0];
    auto error = testcase[i][1];

    // generate input parameter set
    InputParameters params = validParams<EBSDMesh>();
    params.addPrivateParam("_moose_app", _app);
    params.set<std::string>("_object_name") = filename; // use the filename to define a unique name

    // set filename
    params.set<FileName>("filename") = filename;
    params.set<unsigned int>("uniform_refine") = 2;

    // construct mesh object
    std::unique_ptr<EBSDMesh> mesh(new EBSDMesh(params));

    try
    {
      // trigger mesh building with invalid EBSD filename
      mesh->buildMesh();
      FAIL() << "buildMesh should have failed but didn't";
    }
    catch (const std::exception & e)
    {
      std::string msg(e.what());
      EXPECT_TRUE(msg.find(error) != std::string::npos)
          << "case " << i << " file '" << filename << "' failed with unexpected error: " << msg
          << COLOR_DEFAULT << "looking for: " << error << "\n";
    }
  }
}

TEST_F(EBSDMeshErrorTest, geometrySpecifiedError)
{
  // test all these Real parameters
  const unsigned int nreal = 6;
  const char * real_params[nreal] = {"xmin", "xmax", "ymin", "ymax", "zmin", "zmax"};
  testParam<Real>(nreal, real_params, "TestA");

  // test all these int parameters
  const unsigned int nint = 3;
  const char * int_params[nint] = {"nx", "ny", "nz"};
  testParam<unsigned int>(nint, int_params, "TestB");
}

