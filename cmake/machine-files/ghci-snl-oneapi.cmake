# Common settings for our ghci images
include(${CMAKE_CURRENT_LIST_DIR}/ghci-snl.cmake)

# Set kokkos arch
option (Kokkos_ARCH_SPR "" ON)

# Currently, we have 192 cores on blake, but testing 4 ranks is probably enough
set(EKAT_TEST_MAX_RANKS 4 CACHE STRING "Upper limit on ranks for mpi tests")
