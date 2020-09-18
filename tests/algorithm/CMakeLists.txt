include(EkatCreateUnitTest)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ekat_config.f.in
                ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.f)

# Test lin interp processes
set (LIN_INTERP_SRCS
  lin_interp_test.cpp
  lin_interp_ref.f90
)

if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(lin_interp${DP_POSTFIX} "${LIN_INTERP_SRCS}"
    LIBS ekat
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
  target_compile_definitions(lin_interp${DP_POSTFIX} PRIVATE EKAT_TEST_DOUBLE_PRECISION)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(lin_interp${SP_POSTFIX} "${LIN_INTERP_SRCS}"
    LIBS ekat
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
  target_compile_definitions(lin_interp${SP_POSTFIX} PRIVATE EKAT_TEST_SINGLE_PRECISION)
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
    THREADS ${EKAT_TEST_MAX_THREADS}
    EXCLUDE_MAIN_CPP)
  target_compile_definitions(tridiag${DP_POSTFIX} PRIVATE EKAT_TEST_DOUBLE_PRECISION)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(tridiag${SP_POSTFIX} "${TRIDIAG_SRCS}"
    LIBS ekat
    THREADS ${EKAT_TEST_MAX_THREADS}
    EXCLUDE_MAIN_CPP)
  target_compile_definitions(tridiag${SP_POSTFIX} PRIVATE EKAT_TEST_SINGLE_PRECISION)
endif()
