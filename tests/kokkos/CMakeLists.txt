include(EkatCreateUnitTest)

# Test upper_bound
EkatCreateUnitTest(upper_bound upper_bound.cpp
  LIBS ekat::KokkosUtils)

# Test math utils
EkatCreateUnitTest(math_utils math_utils.cpp
  LIBS ekat::KokkosUtils)

# Test view utils
EkatCreateUnitTest(view_utils view_utils.cpp
  LIBS ekat::KokkosUtils)

# Test view broadcast
EkatCreateUnitTest(view_broadcast view_broadcast.cpp
  LIBS ekat::KokkosUtils)

# Test subview utils
EkatCreateUnitTest(subview_utils subview_utils.cpp
  LIBS ekat::KokkosUtils)

# Test team policy utils
EkatCreateUnitTest(team_policy_utils team_policy_utils.cpp
  LIBS ekat::KokkosUtils
  PRINT_OMP_AFFINITY
  THREADS 1 ${EKAT_TEST_MAX_THREADS} ${EKAT_TEST_THREAD_INC}
)

# Test workspace manager
# wsm multithead test is extremely slow when valgrind is on
if (EKAT_ENABLE_VALGRIND)
  set (max_thr 1)
  set (thr_inc 1)
else ()
  set (max_thr ${EKAT_TEST_MAX_THREADS})
  set (thr_inc ${EKAT_TEST_THREAD_INC})
endif()

EkatCreateUnitTest(workspace_mgr workspace_mgr.cpp
  LIBS ekat::KokkosUtils
  PRINT_OMP_AFFINITY
  THREADS 1 ${max_thr} ${thr_inc})

if (Kokkos_ENABLE_CUDA AND Kokkos_ENABLE_CUDA_UVM)
  # Test ability to move a kernel to host
  EkatCreateUnitTest (kernel_on_host kernel_on_host.cpp
    LIBS ekat)
endif()
