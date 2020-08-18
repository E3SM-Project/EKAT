include (${CMAKE_CURRENT_LIST_DIR}/generic.cmake)

# Enable P100 arch in kokkos
set(KOKKOS_ARCH Pascal60 CACHE STRING "")
