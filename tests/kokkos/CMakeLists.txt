include(EkatCreateUnitTest)

if (EKAT_TEST_DOUBLE_PRECISION)
  # Test kokkos utils
  EkatCreateUnitTest(kokkos_utils${DP_POSTFIX} kokkos_utils_tests.cpp
    LIBS ekat
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
    INCLUDE_DIRS "${EKAT_INCLUDE_DIRS}"
  )
  target_compile_definitions(kokkos_utils${DP_POSTFIX} PRIVATE EKAT_TEST_DOUBLE_PRECISION)

  # Test workspace manager
  EkatCreateUnitTest(wsm${DP_POSTFIX} workspace_tests.cpp
    LIBS ekat
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
    INCLUDE_DIRS "${EKAT_INCLUDE_DIRS}"
  )
  target_compile_definitions(wsm${DP_POSTFIX} PRIVATE EKAT_TEST_DOUBLE_PRECISION)
endif ()

if (EKAT_TEST_SINGLE_PRECISION)
  # Test kokkos utils
  EkatCreateUnitTest(kokkos_utils${SP_POSTFIX} kokkos_utils_tests.cpp
    LIBS ekat
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
    INCLUDE_DIRS "${EKAT_INCLUDE_DIRS}"
  )
  target_compile_definitions(kokkos_utils${SP_POSTFIX} PRIVATE EKAT_TEST_SINGLE_PRECISION)

  # Test workspace manager
  EkatCreateUnitTest(wsm${SP_POSTFIX} workspace_tests.cpp
    LIBS ekat
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
    INCLUDE_DIRS "${EKAT_INCLUDE_DIRS}"
  )
  target_compile_definitions(wsm${SP_POSTFIX} PRIVATE EKAT_TEST_SINGLE_PRECISION)
endif ()
