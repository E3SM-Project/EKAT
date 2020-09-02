include (${CMAKE_CURRENT_LIST_DIR}/generic.cmake)

# Enable V100 arch in kokkos
set(Kokkos_ARCH Volta70 CACHE STRING "")
