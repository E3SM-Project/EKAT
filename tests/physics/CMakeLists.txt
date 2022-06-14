include(EkatCreateUnitTest)

configure_file(${PROJECT_SOURCE_DIR}/src/ekat/physics/constants.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/constants.yaml COPYONLY)

EkatCreateUnitTest(read_constants read_constants_tests.cpp
  LIBS ekat )

EkatCreateUnitTest(base_constants constants_tests.cpp
LIBS ekat)

#EkatCreateUnitTest(base_constants_f90 constants_tests.f90
#LIBS ekat)




