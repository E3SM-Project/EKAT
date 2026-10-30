# Common settings for our ghci images
include(${CMAKE_CURRENT_LIST_DIR}/ghci-snl.cmake)

# Enable HIP in kokkos
# NOTE: the GPU arch (Kokkos_ARCH_AMD_GFXXXX) is NOT set here, since it depends on the GPU of
#       the runner. It is passed via cmake options (see the hip job in the ghci-snl-testing workflow).
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/hip.cmake)

set(EKAT_MPI_NP_FLAG "-n" CACHE STRING "The mpirun flag for designating the total number of ranks")

# Currently, we have 2 GPUs/node on the HIP runners, and we run a SINGLE build per node, so we can fit 2 ranks there
set(EKAT_TEST_MAX_RANKS 2 CACHE STRING "Upper limit on ranks for mpi tests")
