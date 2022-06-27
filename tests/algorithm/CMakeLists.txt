include(EkatCreateUnitTest)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ekat_config.f.in
                ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.f)

# Test lin interp processes
set (LIN_INTERP_SRCS
  lin_interp_test.cpp
  lin_interp_ref.F90
)

if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(lin_interp${DP_POSTFIX} "${LIN_INTERP_SRCS}"
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(lin_interp${SP_POSTFIX} "${LIN_INTERP_SRCS}"
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
endif()

# Test tridiag solvers
set (TRIDIAG_SRCS
  tridiag_tests.cpp
  tridiag_bfb.cpp
  tridiag_tests_correctness.cpp
  tridiag_tests_performance.cpp
  tridiag_bfb_mod.F90
)
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(tridiag${DP_POSTFIX} "${TRIDIAG_SRCS}"
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS ${EKAT_TEST_MAX_THREADS}
    EXCLUDE_MAIN_CPP)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(tridiag${SP_POSTFIX} "${TRIDIAG_SRCS}"
    LIBS ekat
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
  PROPERTIES WILL_FAIL TRUE
  LABELS "MustFail"
)
