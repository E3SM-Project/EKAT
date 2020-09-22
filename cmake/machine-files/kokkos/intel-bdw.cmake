include (${CMAKE_CURRENT_LIST_DIR}/generic.cmake)

# Enable Broadwell arch in kokkos
set(KOKKOS_ARCH BDW CACHE STRING "")
