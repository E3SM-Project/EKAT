include(EkatCreateUnitTest)

# Test debug tools
EkatCreateUnitTest(debug_tools debug_tools_tests.cpp
  LIBS Utils)

# Test the framework that checks asserts
if (EKAT_IS_DEBUG_BUILD)
  include(EkatCreateUnitTestWithAsserts)
  EkatCreateUnitTestWithAsserts (assert_tests assert_tests.cpp
    MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  )
endif()

# Ensure that FPE *do* throw when we expect them to
EkatCreateUnitTestExec (fpe_check "fpe_check.cpp")
if (EKAT_ENABLE_FPE_DEFAULT_MASK)
  EkatCreateUnitTestFromExec (fpe_check fpe_check
    WILL_FAIL
    LABELS "MustFail")
else()
  EkatCreateUnitTestFromExec (fpe_check fpe_check)
endif()
