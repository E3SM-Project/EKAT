# EKAT: E3SM Kokkos Applications Toolkit

EKAT is a C/C++ library providing tools and utilities for writing Kokkos-enabled
HPC applications in the E3SM ecosystem. It includes data structures, algorithms,
logging, YAML parsing, and CMake scripts that simplify building and testing
performance-portable code.

Please read our [guidelines](https://github.com/E3SM-Project/EKAT/blob/master/.github/CONTRIBUTING.md)
if you are interested in contributing to EKAT.

This project is licensed under the terms of the BOOST license. See LICENSE.txt
for details.

# Sub-packages

EKAT is organized into the following sub-packages. **Core** is always enabled;
the remaining packages are opt-in (see [Configuring EKAT](#configuring-ekat)).

| Package | CMake target | Description |
|---------|--------------|-------------|
| Core | `ekat::Core` | Foundational utilities: assertions, MPI communicator wrapper, parameter lists, string utilities, scalar traits, std-like meta-utilities, units, and more. Always enabled. |
| KokkosUtils | `ekat::KokkosUtils` | Kokkos-specific utilities: team policies, views, subviews, workspace (kernel-local scratch memory), math helpers, and kernel-level assertions. |
| Pack | `ekat::Pack` | SIMD-friendly `Pack` and `Mask` types to enhance auto-vectorization of loops over short arrays. |
| Algorithm | `ekat::Algorithm` | Kokkos team-level algorithms: linear interpolation of 1D data and solvers for diagonally dominant tridiagonal systems. |
| Expression | `ekat::Expression` | Expression-template infrastructure for lazy evaluation of array operations over Kokkos views. |
| Logging | `ekat::Logging` | spdlog-based logging utilities with MPI-aware and file-sink policies. |
| Parser | `ekat::Parser` | YAML file reader built on top of yaml-cpp, exposing parsed content as an `ekat::ParameterList`. |

There is also a **TestingSupport** component (always built alongside the enabled
packages) that provides Catch2-based test helpers used by EKAT's own test suite
and available to downstream projects.

# Configuring EKAT

EKAT uses CMake (version 3.18 or higher) to generate its build system. Only C
and C++ (C++17) are required; no Fortran compiler is needed.

## Kokkos machine files

To make it easier to configure Kokkos for a specific machine, EKAT ships a
collection of cmake *machine files* under `cmake/machine-files/`:

- `cmake/machine-files/kokkos/` — architecture and threading back-end files
  (e.g., `openmp.cmake`, `cuda.cmake`, `nvidia-v100.cmake`, `intel-skx.cmake`, …)
- `cmake/machine-files/` — full per-machine files that combine architecture and
  threading choices for common E3SM platforms (e.g., `pm-cpu.cmake`,
  `pm-gpu.cmake`, `weaver.cmake`, …)

A custom `my-mach.cmake` file that targets an Intel Skylake CPU with OpenMP
threading could look like:

```cmake
set (EKAT_MACH_FILES_PATH /path/to/ekat-src/cmake/machine-files/kokkos)
include (${EKAT_MACH_FILES_PATH}/openmp.cmake)
include (${EKAT_MACH_FILES_PATH}/intel-skx.cmake)
```

## Example cmake configuration script

The following `do-cmake.sh` script shows how to configure EKAT as a standalone
build, enabling some optional sub-packages and the test suite:

```bash
#!/bin/bash

SRC_DIR=/path/to/ekat-src
BUILD_DIR=/path/to/ekat-build
INSTALL_DIR=/path/to/ekat-install

rm -rf ${BUILD_DIR} && mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake \
  -C /path/to/my-mach.cmake                            \
  \
  -D CMAKE_BUILD_TYPE:STRING=Release                    \
  -D CMAKE_CXX_COMPILER:STRING=g++                      \
  -D CMAKE_INSTALL_PREFIX:PATH=${INSTALL_DIR}           \
  \
  -D EKAT_ENABLE_ALL_PACKAGES:BOOL=ON                   \
  \
  -D EKAT_DISABLE_TPL_WARNINGS:BOOL=ON                  \
  \
  -D EKAT_ENABLE_TESTS:BOOL=ON                          \
  -D EKAT_TEST_MAX_THREADS:STRING=4                     \
  \
  ${SRC_DIR}
```

Once CMake configuration has completed, build and test EKAT with:

```bash
make -j$(nproc)
ctest --output-on-failure
```

## CMake configuration options

| Option | Default | Description |
|--------|---------|-------------|
| `EKAT_ENABLE_ALL_PACKAGES` | `OFF` | Enable all optional sub-packages at once. |
| `EKAT_ENABLE_ALGORITHM` | `OFF` | Enable the Algorithm sub-package. |
| `EKAT_ENABLE_EXPRESSION` | `OFF` | Enable the Expression sub-package. |
| `EKAT_ENABLE_KOKKOS` | `OFF` | Enable the KokkosUtils sub-package. |
| `EKAT_ENABLE_LOGGING` | `OFF` | Enable the Logging sub-package. |
| `EKAT_ENABLE_PACK` | `OFF` | Enable the Pack sub-package. |
| `EKAT_ENABLE_YAML_PARSER` | `OFF` | Enable the Parser sub-package. |
| `EKAT_ENABLE_MPI` | `ON` | Enable MPI support in Core. |
| `EKAT_ENABLE_TESTS` | `ON` | Build the test suite. |
| `EKAT_TEST_MAX_THREADS` | `1` | Maximum number of OpenMP threads used in tests. |
| `EKAT_TEST_LAUNCHER_BUFFER` | `OFF` | Buffer test output and print it all at once (avoids interleaving with concurrent tests). |
| `EKAT_DISABLE_TPL_WARNINGS` | `OFF` | Suppress compiler warnings from third-party libraries. |
| `EKAT_ENABLE_VALGRIND` | `OFF` | Run tests through Valgrind. |
| `EKAT_ENABLE_COMPUTE_SANITIZER` | `OFF` | Run tests through NVIDIA compute-sanitizer. |
| `EKAT_ENABLE_COVERAGE` | `OFF` | Enable code-coverage instrumentation (Debug builds only). |

# CMake utility scripts

EKAT installs several CMake utility scripts that downstream projects can use.
After installing EKAT, add `<install-prefix>/share/cmake/Modules` to
`CMAKE_MODULE_PATH` to access them.

## `EkatCreateUnitTest.cmake`

Provides three functions for creating Catch2-based unit tests:

- **`EkatCreateUnitTestExec(name srcs ...)`** — Creates a test executable from
  source files. Accepts options to set include directories, compiler
  definitions/flags, linked libraries, and linker flags.
- **`EkatCreateUnitTestFromExec(test_name exec_name ...)`** — Takes an existing
  executable and registers one CTest test per requested MPI-rank / OpenMP-thread
  combination (e.g., `MPI_RANKS 1 4` × `THREADS 1 2` produces four tests).
- **`EkatCreateUnitTest(name srcs ...)`** — Convenience wrapper that calls both
  functions above with a combined argument list.

## `EkatSetCompilerFlags.cmake`

Provides macros to initialize and configure compiler flags in a portable way:

- **`ResetFlags()`** — Clears CMake's default language flags for the current
  CMake directory scope, giving you a clean slate.
- **`SetFlags(...)`** / **`SetCommonFlags()`** — Apply recommended flags for
  debug and release builds.
- **`SetProfilingFlags(PROFILER ... COVERAGE ...)`** — Add profiling or coverage
  instrumentation flags.

## `EkatUtils.cmake`

General-purpose CMake helpers:

- **`IsDebugBuild(OUT)`** — Sets `OUT` to `TRUE` when `CMAKE_BUILD_TYPE` is
  `Debug`.
- **`CheckMacroArgs(...)`** — Validates parsed arguments for a macro and emits
  author warnings for unrecognized or missing-value keywords.
- **`EkatDisableAllWarning(target)`** — Suppresses all compiler warnings for a
  given target (useful for third-party code).
- **`dump_cmake_variables([REGEX ...])`** — Prints all visible CMake variables
  to the console, with optional regex filtering.

## `EkatMpiUtils.cmake`

MPI-related helpers:

- **`GetMpiDistributionName(OUT)`** — Detects the MPI distribution (OpenMPI,
  MPICH, Cray, …) by inspecting the MPI header.
- **`DisableMpiCxxBindings()`** — Removes deprecated MPI C++ bindings from
  the link flags to avoid link-time issues.
- **`SetMpiRuntimeEnvVars()`** — Sets runtime environment variables needed by
  the detected MPI distribution.

## `tpls/EkatBuildEkat.cmake`

Lets downstream projects embed EKAT as a subdirectory without worrying about
double-inclusion:

```cmake
include(EkatBuildEkat)
BuildEkat(PREFIX MY_PROJECT)
```

This macro checks a global property to ensure `add_subdirectory` is only called
once, even if multiple sub-components of a project all try to pull in EKAT.

## `tpls/EkatBuildKokkos.cmake` / `tpls/EkatFindKokkos.cmake`

Locate or fetch-and-build Kokkos. `EkatFindKokkos` tries `find_package(Kokkos)`
first; if that fails, `EkatBuildKokkos` fetches Kokkos from the bundled
submodule via `FetchContent`.

## `tpls/EkatBuildYamlCpp.cmake` / `tpls/EkatBuildCatch2.cmake` / `tpls/EkatBuildSpdlog.cmake`

Fetch and build the yaml-cpp, Catch2, and spdlog third-party libraries
respectively via `FetchContent`, using E3SM-Project forks pinned to specific
commits for reproducibility.
