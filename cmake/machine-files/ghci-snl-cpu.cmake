# Common settings for our ghci images
include(${CMAKE_CURRENT_LIST_DIR}/ghci-snl.cmake)

# Currently, we have 32 cores for each ghci-snl-cpu instance, but 4 ranks is enough
set(EKAT_TEST_MAX_RANKS 4 CACHE STRING "Upper limit on ranks for mpi tests")
