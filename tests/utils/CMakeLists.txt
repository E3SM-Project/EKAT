include(EkatCreateUnitTest)

# Test debug tools
if (${EKAT_HAVE_FEENABLEEXCEPT})
EkatCreateUnitTest(debug_tools debug_tools_tests.cpp
  LIBS ekat)
else ()
 message(STATUS "feeenableexcept not found in fenv.h -- disabling debug_tools_tests")
endif()

# Test utilities (c++)
EkatCreateUnitTest(util_cxx util_tests.cpp
  LIBS ekat)

# Test upper_bound
EkatCreateUnitTest(upper_bound upper_bound_test.cpp
  LIBS ekat)

# Test math utils
EkatCreateUnitTest(math_util math_util_tests.cpp
  LIBS ekat)
