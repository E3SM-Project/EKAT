# Let's catch usage of code deprecated in Kokkos 4
option (Kokkos_ENABLE_DEPRECATED_CODE_4 "" OFF)

# We need to manage resources to spread across available cores/gpus
option (EKAT_TEST_LAUNCHER_MANAGE_RESOURCES "" ON)

# Needed by EkatCreateUnitTest
set (EKAT_MPIRUN_EXE "mpirun" CACHE STRING "")
set (EKAT_MPI_NP_FLAG "-n" CACHE STRING "")
