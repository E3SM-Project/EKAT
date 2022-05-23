include(EkatCreateUnitTest)

#EkatCreateUnitTest(physics_constants_f90 constants_tests.f90
#  LIBS ekat)

EkatCreateUnitTest(physics_constants constants_tests.cpp
  LIBS ekat)
