################### BEGIN OF EKAT TESTING OPTIONS ###################

# Set defaults for EKAT testing

#######################
###    THREADING    ###
#######################

if (EKAT_ENABLE_GPU)
  set (DEFAULT_MAX_THREADS 1)
else ()
  set (DEFAULT_MAX_THREADS 16)
endif ()

set (EKAT_TEST_THREAD_INC 1  CACHE STRING "Increment for threads for tests")
set (EKAT_TEST_MAX_THREADS ${DEFAULT_MAX_THREADS} CACHE STRING "Max number of threads for tests")

#######################
###    MPI RANKS    ###
#######################

if (EKAT_ENABLE_MPI)
  set (EKAT_TEST_MPI_EXEC_NAME "mpiexec" CACHE STRING "Name of mpirun executable")
  set (EKAT_TEST_MPI_NP_FLAG "-n" CACHE STRING "Flag to specify number of MPI processes")
  set (EKAT_TEST_MPI_EXTRA_ARGS "--bind-to core" CACHE STRING "Flag to specify extra args for MPI")
  if (EKAT_TEST_MPI_EXEC_NAME)
    set (EKAT_TEST_MAX_RANKS 4 CACHE STRING "Max number of ranks for tests")
  else()
    set (EKAT_TEST_MAX_RANKS 1 CACHE STRING "Max number of ranks for tests")
  endif()
else()
  set (EKAT_TEST_MAX_RANKS 1 CACHE STRING "Max number of ranks for tests")
endif()

#######################
###    PRECISION    ###
#######################

option (EKAT_TEST_SINGLE_PRECISION "Whether tests should be performed in single precision" OFF)
option (EKAT_TEST_DOUBLE_PRECISION "Whether tests should be performed in double precision" ON)

#######################
###     PACKS       ###
#######################

if (EKAT_ENABLE_GPU)
  set (DEFAULT_PACK_SIZE 1)
else ()
  set (DEFAULT_PACK_SIZE 16)
endif ()

set (EKAT_TEST_PACK_SIZE ${DEFAULT_PACK_SIZE} CACHE STRING
  " The number of scalars in a pack::Pack and Mask.
   Larger packs have good performance on conditional-free loops due to improved caching.")

#################### END OF EKAT TESTING OPTIONS ##########################

# This header allows tests to define the type 'Real' based on CPP macros
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ekat_test_config.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/ekat_test_config.h)

# Usually, 'include_directories' is not recommented, and target-specific includes are preferred.
# However, here it is quite handy, since we want the current binary dir to be added to
# the include directories of all tests, so that ekat_test_confif.h can be found.
include_directories (${CMAKE_CURRENT_BINARY_DIR})

# If only one between single and double precision is enabled, there's no need
# to add a suffix to tests names. If, on the other hand, they are both enabled,
# we add sp and dp to distinguish them
if (EKAT_TEST_DOUBLE_PRECISION AND EKAT_TEST_SINGLE_PRECISION)
  set (SP_POSTFIX "_sp")
  set (DP_POSTFIX "_dp")
else()
  set (SP_POSTFIX "")
  set (DP_POSTFIX "")
endif()

#######################
###    Subfolders   ###
#######################

# Core is ALWAYS enabled
add_subdirectory(core)

# Process sub-pacakges as requested
if (EKAT_ENABLE_ALGORITHM)
  add_subdirectory(algorithm)
endif()
if (EKAT_ENABLE_KOKKOS)
  add_subdirectory(kokkos)
endif()
if (EKAT_ENABLE_PACK)
  add_subdirectory(pack)
endif()
if (EKAT_ENABLE_EXPRESSION)
  add_subdirectory(expression)
endif()
if (EKAT_ENABLE_YAML_PARSER)
  add_subdirectory(parser)
endif()
