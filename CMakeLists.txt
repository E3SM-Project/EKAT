cmake_minimum_required(VERSION 3.12)

set (EKAT_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE INTERNAL "")

# Do not ignore <PackageName_ROOT> env vars in find_package() calls
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    cmake_policy (SET CMP0074 NEW)
endif(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")

# Add our own cmake goodies to cmake module search path
list(APPEND CMAKE_MODULE_PATH
     ${EKAT_CMAKE_PATH}
     ${EKAT_CMAKE_PATH}/tpls
)

project (EKAT C CXX)
# EKAT has mostly C++, but (optionally) Fortran too
option (EKAT_ENABLE_FORTRAN "Whether EKAT Fortran support is enabled." ON)
if (EKAT_ENABLE_FORTRAN)
  enable_language(Fortran)
endif()

include(EkatUtils)
IsDebugBuild(EKAT_IS_DEBUG_BUILD)

set (EKAT_VERSION_MAJOR 1)
set (EKAT_VERSION_MINOR 0)
set (EKAT_VERSION_PATCH 0)

# Report the installation prefix.
message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")

###########################
###      EKAT TPLS      ###
###########################

include (EkatTPLs)

############################
###  EKAT CONFIG OPTIONS ###
############################

# TODO: this is never used in EKAT src folder, so it should be removed.
#       Downstream projects should handle bfb logic autonomously
option (EKAT_DEFAULT_BFB "Whether EKAT should default to BFB behavior whenever possible/appropriate." ${EKAT_IS_DEBUG_BUILD})
option (EKAT_ENABLE_FPE "Whether to enable support for floating point exceptions" ON)
if (EKAT_ENABLE_FPE)
  option (EKAT_ENABLE_FPE_DEFAULT_MASK "Whether ekat should set a 'reasonable' FPE mask at startup" OFF)
endif()
option (EKAT_ENABLE_VALGRIND "Whether to run tests with valgrind" OFF)
option (EKAT_ENABLE_CUDA_MEMCHECK "Whether to run tests with cuda-memcheck" OFF)
option (EKAT_ENABLE_COMPUTE_SANITIZER "Whether to run tests with nvidia's compute-sanitizer" OFF)
option (EKAT_ENABLE_COVERAGE "Whether to enable code coverage" OFF)
option (EKAT_TEST_LAUNCHER_BUFFER "Whether test-launcher should buffer all out and print all at once. Useful for avoiding interleaving output when multiple tests are running concurrently" OFF)
option (EKAT_TEST_LAUNCHER_MANAGE_RESOURCES "Whether test-launcher should try to manage thread distribution. Requires a ctest resource file to be effective." OFF)
set (EKAT_PROFILING_TOOL "NONE" CACHE STRING "Profiling tool to be used")

# Path to valgrind suppression file. If none is provided, EKAT will generate one
# for you. The generated ones are OK but there's an element of nondeterminism
# in some of the valgrind interactions with some of our TPLs (like MPI), so
# a persistent, maintained suppression file is best for consistency.
#
# If new valgrind errors emerge that you want to suppress, run the error text
# through gen_sup.sh and add the output to the stored suppression file.
set(EKAT_VALGRIND_SUPPRESSION_FILE "" CACHE FILEPATH "Use this valgrind suppression file if valgrind is enabled.")

# Options to pass to compute-sanitizer, which is a newer, expanded error checker for CUDA programs.
# --error-exitcode 1 will always be added by EKAT.
# We default to the memcheck tool but there are other tools: racecheck, initcheck, and synccheck
# Only a single tool can be enabled at a time
# As an example, if you wanted to enable racecheck for ekat or scream, you'd add this to your cmake cmd:
#   -DEKAT_ENABLE_COMPUTE_SANITIZER=On -DEKAT_COMPUTE_SANITIZER_OPTIONS="--tool racecheck"
set (EKAT_COMPUTE_SANITIZER_OPTIONS "--tool memcheck" CACHE STRING "options to be passed to compute-sanitizer, does nothing if EKAT_ENABLE_COMPUTE_SANITIZER is OFF")

if (EKAT_ENABLE_COVERAGE AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(FATAL_ERROR "Code coverage will only work with Debug build type")
endif()

if (EKAT_DEFAULT_BFB AND NOT EKAT_IS_DEBUG_BUILD)
  message("WARNING: Setting EKAT_DEFAULT_BFB in an optimized build may invalidate BFBness.")
endif()

# MPI CONFIG OPTIONS. Defaults are set for generic MPI desktop.
if (EKAT_ENABLE_MPI)
  set(EKAT_MPIRUN_EXE "mpiexec" CACHE STRING "The executable name for mpirun")
  set(EKAT_MPI_EXTRA_ARGS "--bind-to core" CACHE STRING "Options for mpirun")
  set(EKAT_MPI_NP_FLAG "--map-by" CACHE STRING "The mpirun flag for designating the total number of ranks")
  set(EKAT_MPI_THREAD_FLAG "" CACHE STRING "The mpirun flag for designating the number of threads")
endif()

############################################
###  COMPILER/OS-SPECIFIC CONFIG OPTIONS ###
############################################

# Initialize flags
include (EkatSetCompilerFlags)
ResetFlags()
SetCommonFlags()
SetProfilingFlags(PROFILER ${EKAT_PROFILING_TOOL} COVERAGE ${EKAT_ENABLE_COVERAGE})

include(CheckCXXSymbolExists)
# Most of the FPE stuff was defined in C99, but gnu has some additional non-std
# functions that are helpful (such as feeenableexcept) --- Apple stopped including these
# in 2005, so we have to drop-in replacements for them.
check_cxx_symbol_exists(feenableexcept "fenv.h" EKAT_HAVE_FEENABLEEXCEPT)

##################################
##    EKAT VALGRIND SUPPORT    ###
##################################

if (EKAT_ENABLE_VALGRIND)
  add_subdirectory(valgrind_support)
endif()

############################
###    EKAT LIBRARIES    ###
############################

add_subdirectory(src/ekat)

############################
###     EKAT TESTING     ###
############################

# Set some vars needed to configure test-launcher
if (EKAT_ENABLE_MPI)
  SetMpiRuntimeEnvVars()
endif()

#
# These need to be set to python boolean values
#
if (EKAT_ENABLE_GPU)
  set (TEST_LAUNCHER_ON_GPU True)
else()
  set (TEST_LAUNCHER_ON_GPU False)
endif()

if (EKAT_TEST_LAUNCHER_MANAGE_RESOURCES)
  set (TEST_LAUNCHER_MANAGE_RESOURCES True)
else()
  set (TEST_LAUNCHER_MANAGE_RESOURCES False)
endif()

# Configure/install test-launcher to build/install folder
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/test-launcher
               ${CMAKE_BINARY_DIR}/bin/test-launcher)
include(GNUInstallDirs)
install (PROGRAMS ${CMAKE_BINARY_DIR}/bin/test-launcher
         DESTINATION ${CMAKE_INSTALL_BINDIR})

# By default, we DO build ekat tests. This may change in the future
option (EKAT_ENABLE_TESTS "Whether tests should be built." ON)

if (EKAT_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (EKAT_ENABLE_CUDA_MEMCHECK AND NOT Kokkos_ENABLE_CUDA)
  message(FATAL_ERROR "Makes no sense to turn on cuda-memcheck without CUDA")
endif()

if (EKAT_ENABLE_CUDA_MEMCHECK AND EKAT_ENABLE_VALGRIND)
  message(FATAL_ERROR "Cannot simultanously enable valgrind and cuda-memcheck")
endif()

###########################################
###   Package ekat as a CMake package   ###
###########################################

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

CONFIGURE_PACKAGE_CONFIG_FILE(
  cmake/EkatConfig.cmake.in
  "${EKAT_BINARY_DIR}/EkatConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cmake)

WRITE_BASIC_PACKAGE_VERSION_FILE("${EKAT_BINARY_DIR}/EkatConfigVersion.cmake"
    VERSION ${EKAT_VERSION_MAJOR}.${EKAT_VERSION_MINOR}.${EKAT_VERSION_PATCH}.
    COMPATIBILITY SameMajorVersion)

# Install the EkatConfig*.cmake files
install(FILES
  "${EKAT_BINARY_DIR}/EkatConfig.cmake"
  "${EKAT_BINARY_DIR}/EkatConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ekat)

# Install cmake targets
install(EXPORT EkatTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ekat)

###########################################
###   Install ekat cmake util scripts   ###
###########################################

install (FILES
  ${EKAT_CMAKE_PATH}/EkatCreateUnitTest.cmake
  ${EKAT_CMAKE_PATH}/EkatSetCompilerFlags.cmake
  ${EKAT_CMAKE_PATH}/EkatUtils.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules)

install (DIRECTORY ${EKAT_CMAKE_PATH}/tpls
         DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules)
