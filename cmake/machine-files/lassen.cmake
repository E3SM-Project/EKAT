# Load v100 arch and cuda backend for kokkos
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/nvidia-v100.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/kokkos/cuda.cmake)

include (${CMAKE_CURRENT_LIST_DIR}/mpi/other.cmake)

set(EKAT_MPIRUN_EXE "jsrun -E LD_PRELOAD=/opt/ibm/spectrum_mpi/lib/pami_471/libpami.so" CACHE STRING "")
set(EKAT_MPI_NP_FLAG "--np" CACHE STRING "")
