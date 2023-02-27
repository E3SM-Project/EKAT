# Load skx arch and openmp backend for kokkos
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/intel-skx.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/openmp.cmake)

# Blake cannot use srun for some reason and therefore needs EKAT
# to manage resources.
set (EKAT_TEST_LAUNCHER_MANAGE_RESOURCES True CACHE BOOL "")
