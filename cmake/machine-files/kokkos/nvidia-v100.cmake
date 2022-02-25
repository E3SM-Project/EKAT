include (${CMAKE_CURRENT_LIST_DIR}/generic.cmake)

# Enable V100 arch in kokkos
option(Kokkos_ARCH_VOLTA70 "" ON)

# This var is needed by relatively recent CMake when CUDA language is enabled
# If not defined, CMake issues a warning
set (CMAKE_CUDA_ARCHITECTURES 70 CACHE STRING "")
