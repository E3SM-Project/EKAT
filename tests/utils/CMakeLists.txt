include(EkatCreateUnitTest)

# Test catch main options

EkatCreateUnitTest (catch_main_tests catch_main_tests.cpp
  EXE_ARGS "--args -a 1 --b 2 3 -c=4,5 --d 6,7 -K -foo --bar"
  PROPERTIES PASS_REGULAR_EXPRESSION
  ".*flags: K bar foo, params: a=1, vec_params: a=1 b=2,3 c=4,5 d=6,7")

# Test debug tools
EkatCreateUnitTest(debug_tools debug_tools_tests.cpp
  LIBS ekat)

# Test std_meta stuff
EkatCreateUnitTest(std_meta std_meta.cpp
  LIBS ekat)

# Test meta utilities
EkatCreateUnitTest(meta_utils meta_utils.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(util_cxx util_tests.cpp
  LIBS ekat)

# Test string utilities
EkatCreateUnitTest(string_utils string_utils_tests.cpp
  LIBS ekat)

# Test upper_bound
EkatCreateUnitTest(upper_bound upper_bound_test.cpp
  LIBS ekat)

# Test factory
EkatCreateUnitTest(factory factory.cpp
  LIBS ekat)

# Test math utils
EkatCreateUnitTest(math_util math_util_tests.cpp
  LIBS ekat)

# Test that failure are indeed spotted. Also, verify that redirection
# of input is not parsed as test filter, by fwding a file
EkatCreateUnitTest(regress_fail regress_fail.cpp
  LIBS ekat EXE_ARGS " < CTestTestfile.cmake"
  WILL_FAIL
  LABELS "MustFail"
)

# Test that Catch returns a failure if invalid flags are passed
EkatCreateUnitTestFromExec(catch_main_invalid_flags regress_fail
  EXE_ARGS " --non-existent-flag"
  WILL_FAIL
  LABELS "MustFail"
)

# Ensure that FPE *do* throw when we expect them to
EkatCreateUnitTestExec (fpe_check "fpe_check.cpp")
if (EKAT_ENABLE_FPE_DEFAULT_MASK)
  EkatCreateUnitTestFromExec (fpe_check fpe_check
    WILL_FAIL
    LABELS "MustFail")
else()
  EkatCreateUnitTestFromExec (fpe_check fpe_check)
endif()

# Test the framework that checks asserts
if (EKAT_IS_DEBUG_BUILD)
  include(EkatCreateUnitTestWithAsserts)
  EkatCreateUnitTestWithAsserts (assert_tests assert_tests.cpp
    MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  )
endif()
