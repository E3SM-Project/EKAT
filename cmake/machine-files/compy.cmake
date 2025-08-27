# Load epyc arch and openmp backend for kokkos
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/intel-skx.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/openmp.cmake)

set(EKAT_MPI_NP_FLAG "-p short --mpi=pmi2 -n" CACHE STRING "" FORCE)
