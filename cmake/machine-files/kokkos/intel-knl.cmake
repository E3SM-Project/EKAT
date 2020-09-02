include (${CMAKE_CURRENT_LIST_DIR}/generic.cmake)

# Enable KNL arch in kokkos
set(Kokkos_ARCH KNL CACHE STRING "")
