include(EkatCreateUnitTest)

# Test units framework
EkatCreateUnitTest(units "units.cpp" LIBS ekat)

# Test physical constants
EkatCreateUnitTest(physical_constants  "physical_constants.cpp" LIBS ekat)
