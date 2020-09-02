include (${CMAKE_CURRENT_LIST_DIR}/generic.cmake)

# Enable P100 arch in kokkos
option(Kokkos_ARCH_PASCAL60 "" ON)
