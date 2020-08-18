include (${CMAKE_CURRENT_LIST_DIR}/generic.cmake)

# Enable KNL arch in kokkos
set(KOKKOS_ARCH KNL CACHE STRING "")
