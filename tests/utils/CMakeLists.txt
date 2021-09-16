include(EkatCreateUnitTest)

# Test debug tools
EkatCreateUnitTest(debug_tools debug_tools_tests.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(util_cxx util_tests.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(string_utils string_utils_tests.cpp
  LIBS ekat)

# Test upper_bound
EkatCreateUnitTest(upper_bound upper_bound_test.cpp
  LIBS ekat)

# Test math utils
EkatCreateUnitTest(math_util math_util_tests.cpp
  LIBS ekat)

# Test that failure are indeed spotted. Also, verify that redirection
# of input is not parsed as test filter, by fwding a file
EkatCreateUnitTest(regress_fail regress_fail.cpp
  LIBS ekat EXE_ARGS " < CTestTestfile.cmake"
  PROPERTIES WILL_FAIL true
  )
