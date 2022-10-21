cmake_minimum_required(VERSION 3.3)

set (EKAT_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE INTERNAL "")

# Do not ignore <PackageName_ROOT> env vars in find_package() calls
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    cmake_policy (SET CMP0074 NEW)
endif(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")

# Add our own cmake goodies to cmake module search path
list(APPEND CMAKE_MODULE_PATH
     ${EKAT_CMAKE_PATH}
     ${EKAT_CMAKE_PATH}/pkg_build
)

# Scripts containing macros needed to handle TPLs
include(EkatBuildKokkos)
include(EkatBuildYamlCpp)
include(EkatBuildSpdlog)

# EKAT has mostly C++, but some Fortran too
project (EKAT C CXX Fortran)

# MPI is enabled by default, but not needed.
option (EKAT_ENABLE_MPI "Whether EKAT requires MPI." ON)

if (EKAT_ENABLE_MPI)
  find_package(MPI REQUIRED COMPONENTS C)

  # NOTE: may be an overkill, but depending on which FindMPI module is called,
  #       if _FIND_REQUIRED is not checked, we may not get a fatal error
  #       if the required components are not found. So check the _FOUND var.
  if (NOT MPI_C_FOUND)
    message (FATAL_ERROR "EKAT *requires* the C component of MPI to be found")
  endif()

  # We should avoid cxx bindings in mpi; they are already deprecated,
  # and can cause headaches at link time, cause they require -lmpi_cxx
  # (for openpmi; -lmpicxx for mpich) flag.
  include(EkatMpiUtils)
  DisableMpiCxxBindings()

  # MPI-related options
  option (EKAT_MPI_ERRORS_ARE_FATAL " Whether EKAT should crash when MPI errors happen." ON)
endif()

include(EkatUtils)
IsDebugBuild(EKAT_IS_DEBUG_BUILD)

set (EKAT_VERSION_MAJOR 1)
set (EKAT_VERSION_MINOR 0)
set (EKAT_VERSION_PATCH 0)

# Report the installation prefix.
message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")

############################
###  EKAT CONFIG OPTIONS ###
############################

option (EKAT_DEFAULT_BFB "Whether EKAT should default to BFB behavior whenever possible/appropriate." ${EKAT_IS_DEBUG_BUILD})
option (EKAT_ENABLE_FPE "Whether to enable support for floating point exceptions" ON)
if (EKAT_ENABLE_FPE)
  option (EKAT_ENABLE_FPE_DEFAULT_MASK "Whether ekat should set a 'reasonable' FPE mask at startup" OFF)
endif()
option (EKAT_ENABLE_VALGRIND "Whether to run tests with valgrind" OFF)
option (EKAT_ENABLE_CUDA_MEMCHECK "Whether to run tests with cuda-memcheck" OFF)
option (EKAT_ENABLE_COVERAGE "Whether to enable code coverage" OFF)
option (EKAT_TEST_LAUNCHER_BUFFER "Whether test-launcher should buffer all out and print all at once. Useful for avoiding interleaving output when multiple tests are running concurrently" OFF)
set (EKAT_PROFILING_TOOL "NONE" CACHE STRING "Profiling tool to be used")

if (EKAT_ENABLE_COVERAGE AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(FATAL_ERROR "Code coverage will only work with Debug build type")
endif()

if (EKAT_DEFAULT_BFB AND NOT EKAT_IS_DEBUG_BUILD)
  message("WARNING: Setting EKAT_DEFAULT_BFB in an optimized build may invalidate BFBness.")
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

###########################
###      EKAT TPLS      ###
###########################

option (EKAT_DISABLE_TPL_WARNINGS "Whether we should suppress warnings when compiling TPLs." OFF)

# WARNING: you CANNOT do list(APPEND var item1 ... item2) if var is a CACHE variable!
# Therefore, use an internal var during tpl parsing, then set a cache var ONCE at the end
set (EKAT_TPL_LIBRARIES_INTERNAL)

# A good chunk of EKAT is a utility layer over kokkos
# Note: for SYCL/HIP backend, bleeding edge of kokkos/develop might be needed to test some functionality
# Kokkos_ROOT can point to custom installation directory
if (Kokkos_ROOT)
  find_package(Kokkos REQUIRED)
  set(Kokkos_LIBRARIES Kokkos::kokkos)
else()
  BuildKokkos()
endif()
  list (APPEND EKAT_TPL_LIBRARIES_INTERNAL ${Kokkos_LIBRARIES})

set (EKAT_ENABLE_GPU False)
if (Kokkos_ENABLE_CUDA OR Kokkos_ENABLE_HIP OR Kokkos_ENABLE_SYCL)
  set (EKAT_ENABLE_GPU True)
endif ()

# EKAT also has some yaml parsing utility
BuildYamlcpp()
list (APPEND EKAT_TPL_LIBRARIES_INTERNAL ${YAMLCPP_LIBRARIES})

# EKAT uses the spdlog tpl to handle logging
BuildSpdlog()
list (APPEND EKAT_TPL_LIBRARIES_INTERNAL ${SPDLOG_LIBRARIES})

set (EKAT_TPL_LIBRARIES ${EKAT_TPL_LIBRARIES_INTERNAL} CACHE INTERNAL "List of EKAT's TPLs")

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
if (EKAT_ENABLE_GPU)
  set (TEST_LAUNCHER_ON_GPU True)
else()
  set (TEST_LAUNCHER_ON_GPU False)
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

install (DIRECTORY ${EKAT_CMAKE_PATH}/pkg_build
         DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules)
