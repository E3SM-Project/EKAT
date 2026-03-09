include(EkatCreateUnitTest)

# Test linear interpolation
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(lin_interp${DP_POSTFIX}
    SOURCES lin_interp_test.cpp
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(lin_interp${SP_POSTFIX}
    SOURCES lin_interp_test.cpp
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
endif()

# Test tridiag solver
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(tridiag${DP_POSTFIX}
    SOURCES tridiag_tests.cpp
            tridiag_tests_correctness.cpp
            tridiag_tests_performance.cpp
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS ${EKAT_TEST_MAX_THREADS}
    EXCLUDE_MAIN_CPP)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(tridiag${SP_POSTFIX}
    SOURCES tridiag_tests.cpp
            tridiag_tests_correctness.cpp
            tridiag_tests_performance.cpp
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS ${EKAT_TEST_MAX_THREADS}
    EXCLUDE_MAIN_CPP)
endif()

# Check that the tridiags main returns nonzero if invalid flags are passed
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTestFromExec(tridiag_invalid_flags tridiag${SP_POSTFIX}
    EXE_ARGS "--non-existent-flag"
    LABELS "MustFail"
    WILL_FAIL
  )
else()
  EkatCreateUnitTestFromExec(tridiag_invalid_flags tridiag${DP_POSTFIX}
    EXE_ARGS "--non-existent-flag"
    LABELS "MustFail"
    WILL_FAIL
  )
endif()

# Test reduction utilities
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(reduction${DP_POSTFIX}
    SOURCES reduction_tests.cpp
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    LIBS ekat::Algorithm)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(reduction${SP_POSTFIX}
    SOURCES reduction_tests.cpp
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    LIBS ekat::Algorithm)
endif()
