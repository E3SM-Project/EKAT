cmake_minimum_required(VERSION 3.3)

set (EKAT_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE INTERNAL "")

# Add out own cmake goodies to cmake module search path
list(APPEND CMAKE_MODULE_PATH ${EKAT_CMAKE_PATH})

# We should avoid cxx bindings in mpi; they are already deprecated,
# and can cause headaches at link time, cause they require -lmpi_cxx
# (for openpmi; -lmpicxx for mpich) flag.
include(EkatMpiUtils)
DisableMpiCxxBindings()

# Scripts containing macros needed to handle TPLs
include(${EKAT_CMAKE_PATH}/Kokkos.cmake)
include(${EKAT_CMAKE_PATH}/YamlCpp.cmake)

# Note: we need Kokkos_SOURCE_DIR to be defined *before* calling EkatSetNvccWrapper.
EkatSetKokkosSourceDir()

# If building on cuda, use ekat_mpicxx compiler, which sets nvcc_wrapper as backend compiler
if (Kokkos_ENABLE_CUDA)
  include (EkatSetNvccWrapper)
  EkatSetNvccWrapper()
endif()

# EKAT has mostly C++, but some Fortran too
project (EKAT CXX Fortran)

# Report the installation prefix.
message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")

###########################
###      EKAT TPLS      ###
###########################

option (EKAT_DISABLE_TPL_WARNINGS "Whether we should suppress warnings when compiling TPLs." OFF)

# WARNING: you CANNOT do list(APPEND var item1 ... item2) if var is a CACHE variable!
# Therefore, use an internal var during tpl parsing, then set a cache var ONCE at the end
set (EKAT_TPL_INCLUDE_DIRS_INTERNAL)
set (EKAT_TPL_LIBRARIES_INTERNAL)

# A good chunk of EKAT is a utility layer over kokkos
EkatBuildKokkos()
list (APPEND EKAT_TPL_INCLUDE_DIRS_INTERNAL ${Kokkos_INCLUDE_DIRS})
list (APPEND EKAT_TPL_LIBRARIES_INTERNAL ${Kokkos_LIBRARIES})

# EKAT also has some yaml parsing utility
EkatBuildYamlcpp()
list (APPEND EKAT_TPL_INCLUDE_DIRS_INTERNAL ${YAMLCPP_INCLUDE_DIRS})
list (APPEND EKAT_TPL_LIBRARIES_INTERNAL ${YAMLCPP_LIBRARIES})

set (EKAT_TPL_LIBRARIES ${EKAT_TPL_LIBRARIES_INTERNAL} CACHE INTERNAL "List of EKAT's TPLs")
set (EKAT_TPL_INCLUDE_DIRS ${EKAT_TPL_INCLUDE_DIRS_INTERNAL} CACHE INTERNAL "List of EKAT's TPLs include directories")

############################
###  EKAT CONFIG OPTIONS ###
############################

option (EKAT_MPI_ERRORS_ARE_FATAL " Whether EKAT should crash when MPI errors happen." ON)
option (EKAT_DEFAULT_BFB "Whether EKAT should default to BFB behavior whenever possible/appropriate." OFF)

####################################
###  MAC-SPECIFIC CONFIG OPTIONS ###
####################################

include(CheckSymbolExists)
# Most of the FPE stuff was defined in c++99, but gnu has some additional non-std
# functions that are helpful (such as feeenableexcept) --- Apple stopped including these
# in 2005, so we have to drop-in replacements for them.
check_symbol_exists(feenableexcept "fenv.h" EKAT_HAVE_FEENABLEEXCEPT)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  #  GCC has already impl'd some c++14 features even in a c++11 build.  Apple's Clang
  # (perhaps clang generally) doesn't like this and requires -std=c++14 for these features.
  set(CMAKE_CXX_STANDARD 14)
endif()

############################
###     EKAT LIBRARY     ###
############################

# This is the only way to expose ekat's include dirs to host projects (if any)
set (EKAT_INCLUDE_DIRS_LIST
  ${EKAT_SOURCE_DIR}/src
  ${EKAT_BINARY_DIR}
  ${EKAT_BINARY_DIR}/src/ekat/ekat_f90_modules
  ${EKAT_TPL_INCLUDE_DIRS}
)

set (EKAT_INCLUDE_DIRS "${EKAT_INCLUDE_DIRS_LIST}" CACHE INTERNAL "EKAT library include dirs")

include(EkatSetCompilerFlags)
SetCompilerFlags()

add_subdirectory(src/ekat)

############################
###     EKAT TESTING     ###
############################

# By default, we DO build ekat tests. This may change in the future
option (EKAT_ENABLE_TESTS "Whether tests should be built." ON)

if (EKAT_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Generate configuration file
include (EkatUtils)
EkatConfigFile (${EKAT_SOURCE_DIR}/ekat_config.h.in
                ${EKAT_BINARY_DIR}/ekat_config.h
                F90_FILE ${EKAT_BINARY_DIR}/ekat_config.f)
install(FILES ${EKAT_BINARY_DIR}/ekat_config.h
              ${EKAT_BINARY_DIR}/ekat_config.f
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ekat)
