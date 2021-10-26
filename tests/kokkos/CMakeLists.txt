include(EkatCreateUnitTest)

if (EKAT_TEST_DOUBLE_PRECISION)
  # Test kokkos utils
  EkatCreateUnitTest(kokkos_utils${DP_POSTFIX} kokkos_utils_tests.cpp
    LIBS ekat
    PRINT_OMP_AFFINITY
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
  )

  # Test workspace manager
  EkatCreateUnitTest(wsm${DP_POSTFIX} workspace_tests.cpp
    LIBS ekat
    PRINT_OMP_AFFINITY
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
  )
endif ()

if (EKAT_TEST_SINGLE_PRECISION)
  # Test kokkos utils
  EkatCreateUnitTest(kokkos_utils${SP_POSTFIX} kokkos_utils_tests.cpp
    LIBS ekat
    PRINT_OMP_AFFINITY
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
  )

  # Test workspace manager
  EkatCreateUnitTest(wsm${SP_POSTFIX} workspace_tests.cpp
    LIBS ekat
    PRINT_OMP_AFFINITY
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
  )
endif ()
