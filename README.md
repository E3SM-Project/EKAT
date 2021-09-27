# EKAT: E3SM Kokkos Applications Toolkit

EKAT is a small library, containing tools and libraries for writing Kokkos-enabled HPC C++ in the E3SM ecosystem.
Tools include C++/Fortran code (data structures, functions, macros) as well as CMake scripts.

Please, read our [guidelines](https://github.com/E3SM-Project/EKAT/blob/master/.github/CONTRIBUTING.md) if you are interested
in contributing to EKAT.

# Overview

Despite being first created with the goal of being a common implementation for common Kokkos-related kernels,
EKAT evolved to contain a variety of C++ and CMake utilities. Here is a list of some of the utilities provided
in EKAT.

- C++ structures to enhance vectorization (Pack and Mask).
- C++ structures for kernel-local scratch memory (Workspace).
- C++ interface to read and store content of yaml files.
- C++ routines for team-level solvers for diagonally dominant tridiagonal systems.
- C++ routines for team-level efficient linear interpolation of 1d data.
- Backport of some C++17 features (such as std::any), and several std-like meta-utilities.
- CMake utilities for creating a suite of unit tests for multiple MPI/OpenMP rank/threads combinations.
- CMake machine files to configure Kokkos for common architectures and threading framework.

# Configuring EKAT

EKAT uses CMake to generate the build system. To make life easier, we provide some machine files
for the setup of Kokkos properties, located in cmake/machine-files/kokkos. In particular, we provide
machine files for the architecture specs, as well as machine files for the threading framework.
In cmake/machine-files you can see some example o f how to combine architecture/threading machine
files into a single one, which can later be used in your cmake configuration script.

For instance, the following `my-mach.cmake` file combines intel Skylake arch specs with a OpenMP theading backend.

```
set (EKAT_MACH_FILES_PATH /path/to/ekat-src/cmake/machine-files/kokkos)
include (${EKAT_MACH_FILES_PATH}/openmp.cmake)
include (${EKAT_MACH_FILES_PATH}/intel-skx.cmake)

```
And this `do-cmake.sh` script would configure EKAT for a debug build using the machine file above

```
#!/bin/bash

SRC_DIR=${WORK_DIR}/libs/ekat/ekat-src/branch
INSTALL_DIR=${WORK_DIR}/libs/ekat/ekat-install/branch

rm -rf CMakeFiles
rm -f  CMakeCache.txt

cmake \
  -C /path/to/my-mach.cmake                         \
  \
  -D CMAKE_BUILD_TYPE:STRING=DEBUG                  \
  -D CMAKE_CXX_COMPILER:STRING=mpicxx               \
  -D CMAKE_Fortran_COMPILER:STRING=mpifort          \
  -D CMAKE_INSTALL_PREFIX:PATH=${INSTALL_DIR}       \
  \
  -D EKAT_DISABLE_TPL_WARNINGS:BOOL=ON              \
  \
  -D EKAT_ENABLE_TESTS:BOOL=ON                      \
  -D EKAT_TEST_DOUBLE_PRECISION:BOOL=ON             \
  -D EKAT_TEST_SINGLE_PRECISION:BOOL=ON             \
  -D EKAT_TEST_MAX_THREADS:STRING=8                 \
  \
  ${SRC_DIR}
```

Here are some of EKAT's cmake config options:

- EKAT_ENABLE_TESTS: enables ekat testing
- EKAT_TEST_[DOUBLE|SINGLE]_PRECISION: whether to test single and/or double precision.
- EKAT_TEST_MAX_[THREADS|RANKS]: maximum number of threads/ranks to use in unit tests
- EKAT_TEST_THREADS_INC: increment in number of threads for each test.
- EKAT_DEFAULT_BFB: in certain kernels, whether to default to a BFB, serialized, implementation.
- EKAT_DISABLE_TPL_WARNINGS: whether warnings from TPLs should be disabled.
- EKAT_ENABLE_VALGRIND: whether tests should be run through valgrind.
- EKAT_ENABLE_CUDA_MEMCHECK: whether tests should be run through cuda-memcheck.
- EKAT_ENABLE_COVERAGE: whether to enable code coverage in the compiler.

Once CMake configuration has completed, you can build and test EKAT with the usual `make` and `ctest` commands.
