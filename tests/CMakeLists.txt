################### BEGIN OF EKAT TESTING OPTIONS ###################

# Set defaults for EKAT testing
# Note: Determine some of the defaults based on Kokkos backend

#######################
###    THREADING    ###
#######################

if (Kokkos_ENABLE_CUDA)
  set (DEFAULT_MAX_THREADS 1)
else ()
  set (DEFAULT_MAX_THREADS 16)
endif ()

set (EKAT_TEST_THREAD_INC 1  CACHE STRING "Increment for threads for tests")
set (EKAT_TEST_MPI_EXEC_NAME "mpiexec" CACHE STRING "Name of mpirun executable")
set (EKAT_TEST_MPI_NP_FLAG "-n" CACHE STRING "Flag to specify number of MPI processes")
set (EKAT_TEST_MPI_EXTRA_ARGS "--bind-to core" CACHE STRING "Flag to specify extra args for MPI")
if (EKAT_TEST_MPI_EXEC_NAME)
  set (EKAT_TEST_MAX_RANKS 4 CACHE STRING "Max number of ranks for tests")
else()
  set (EKAT_TEST_MAX_RANKS 1 CACHE STRING "Max number of ranks for tests")
endif()
set (EKAT_TEST_MAX_THREADS ${DEFAULT_MAX_THREADS} CACHE STRING "Max number of threads for tests")

#######################
###    PRECISION    ###
#######################

option (EKAT_TEST_SINGLE_PRECISION "Whether tests should be performed in single precision" OFF)
option (EKAT_TEST_DOUBLE_PRECISION "Whether tests should be performed in double precision" ON)

#######################
###     PACKS       ###
#######################

if (Kokkos_ENABLE_CUDA)
  set (DEFAULT_PACK_SIZE 1)
  set (DEFAULT_SMALL_PACK_SIZE 1)
else ()
  set (DEFAULT_PACK_SIZE 16)
  set (DEFAULT_SMALL_PACK_SIZE 8)
endif ()

set (EKAT_TEST_PACK_SIZE       ${DEFAULT_PACK_SIZE} CACHE STRING
  " The number of scalars in a pack::Pack and Mask.
   Larger packs have good performance on conditional-free loops due to improved caching.")
set (EKAT_TEST_SMALL_PACK_SIZE  ${DEFAULT_SMALL_PACK_SIZE} CACHE STRING
  " The number of scalars in a pack::SmallPack and SmallMask. Smaller packs can have better performance in loops with conditionals, since more of the packs will have masks with uniform value.")

# For some routines, SKX may have better performance with pksize=1
# The following lines are meant to set up another variable to store
# this pack size, which is usually equal to EKAT_TEST_PACK_SIZE,
# but may be equal to 1 if EKAT_TEST_POSSIBLY_NO_PACK is set to TRUE
# For example, by default we trigger this special behavior on SKX
if (Kokkos_ARCH_SKX)
  set(DEFAULT_POSSIBLY_NO_PACK TRUE)
  set (DEFAULT_POSSIBLY_NO_PACK_SIZE 1)
else()
  set(DEFAULT_POSSIBLY_NO_PACK FALSE)
set (DEFAULT_POSSIBLY_NO_PACK_SIZE ${EKAT_TEST_PACK_SIZE})
endif ()

set(EKAT_TEST_POSSIBLY_NO_PACK ${DEFAULT_POSSIBLY_NO_PACK} CACHE BOOL
  " Set possibly-no-pack to this value. You can set it to something else to restore packs on SKX for testing.")
set (EKAT_TEST_POSSIBLY_NO_PACK_SIZE ${DEFAULT_POSSIBLY_NO_PACK_SIZE} CACHE STRING "")

#######################
###      MISC       ###
#######################

option (EKAT_TEST_STRICT_FP " Whether EKAT tests should adopt a strict fp model testing." ON)

#################### END OF EKAT TESTING OPTIONS ##########################

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

# Create test config.h file
include (EkatUtils)
EkatConfigFile (${CMAKE_CURRENT_SOURCE_DIR}/ekat_test_config.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/ekat_test_config.h)

# Usually, you should not use 'include_directories', and prefer target-specific includes.
# However, here it is quite handy, since we want the current binary dir to be added to
# the include directories of all tests, so that ekat_test_confif.h can be found.
include_directories (${CMAKE_CURRENT_BINARY_DIR})

# Process all subdirs
add_subdirectory(algorithm)
add_subdirectory(io)
add_subdirectory(kokkos)
add_subdirectory(mpi)
add_subdirectory(pack)
add_subdirectory(units)
add_subdirectory(utils)
add_subdirectory(logging)
