# Common settings for our ghci images
include(${CMAKE_CURRENT_LIST_DIR}/ghci-snl.cmake)

# Enable CUDA in kokkos
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/cuda.cmake)

set(EKAT_MPI_NP_FLAG "-n" CACHE STRING "The mpirun flag for designating the total number of ranks")

# Currently, we have 2 GPUs/node on Blake, and we run a SINGLE build per node, so we can fit 2 ranks there
set(SCREAM_TEST_MAX_RANKS 2 CACHE STRING "Upper limit on ranks for mpi tests")
