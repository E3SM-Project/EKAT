include(EkatCreateUnitTest)

# Test debug tools
EkatCreateUnitTest(debug_tools debug_tools_tests.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(util_cxx util_tests.cpp
  LIBS ekat)

# Test upper_bound
EkatCreateUnitTest(upper_bound upper_bound_test.cpp
  LIBS ekat)
