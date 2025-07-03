include(EkatCreateUnitTest)

# Test linear interpolation
set (LIN_INTERP_SRCS
  lin_interp_test.cpp
)

if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(lin_interp${DP_POSTFIX} "${LIN_INTERP_SRCS}"
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(lin_interp${SP_POSTFIX} "${LIN_INTERP_SRCS}"
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
endif()

# Test tridiag solver
set (TRIDIAG_SRCS
  tridiag_tests.cpp
  tridiag_tests_correctness.cpp
  tridiag_tests_performance.cpp
)

if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(tridiag${DP_POSTFIX} "${TRIDIAG_SRCS}"
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS ${EKAT_TEST_MAX_THREADS}
    EXCLUDE_MAIN_CPP)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(tridiag${SP_POSTFIX} "${TRIDIAG_SRCS}"
    LIBS ekat::Algorithm
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS ${EKAT_TEST_MAX_THREADS}
    EXCLUDE_MAIN_CPP)
endif()

# Check that the tridiags main returns nonzero if invalid flags are passed
if (EKAT_TEST_SINGLE_PRECISION)
  set (invalid_flags_exec tridiag${SP_POSTFIX})
else()
  set (invalid_flags_exec tridiag${DP_POSTFIX})
endif()

EkatCreateUnitTestFromExec(tridiag_invalid_flags ${invalid_flags_exec}
  EXE_ARGS "--non-existent-flag"
  LABELS "MustFail"
  WILL_FAIL
)

# Test reduction utilities
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(reduction${DP_POSTFIX} reduction_tests.cpp
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    LIBS ekat::Algorithm)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(reduction${SP_POSTFIX} reduction_tests.cpp
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    LIBS ekat::Algorithm)
endif()
