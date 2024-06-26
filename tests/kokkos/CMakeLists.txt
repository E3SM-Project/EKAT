include(EkatCreateUnitTest)

if (EKAT_TEST_DOUBLE_PRECISION)
  # Test kokkos utils
  EkatCreateUnitTest(kokkos_utils${DP_POSTFIX} kokkos_utils_tests.cpp
    LIBS ekat
    PRINT_OMP_AFFINITY
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
  )

  # Test workspace manager
  if (EKAT_ENABLE_VALGRIND)
    # wsm multithead test is extremely slow when valgrind is on
    EkatCreateUnitTest(wsm${DP_POSTFIX} workspace_tests.cpp
      LIBS ekat
      PRINT_OMP_AFFINITY
      COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
      THREADS 1 1)
  else()
    EkatCreateUnitTest(wsm${DP_POSTFIX} workspace_tests.cpp
      LIBS ekat
      PRINT_OMP_AFFINITY
      COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
      THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC})
  endif()
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
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
  )
endif ()

if (Kokkos_ENABLE_CUDA AND Kokkos_ENABLE_CUDA_UVM)
  # Test ability to move a kernel to host
  EkatCreateUnitTest (kernel_on_host kernel_on_host.cpp
    LIBS ekat)
endif()
