# Where ekat's cmake scripts live
set (EKAT_CMAKE_PATH ${CMAKE_CURRENT_LIST_DIR}/../ CACHE INTERNAL "")

# This cmake utility allows you to build ekat just by adding
#
#   Include(Ekat)
#   BuildEkat(/path/to/ekat/source)
#
# to your CMakeLists.txt. The advantage over manually adding the
# ekat subdirectory is that this script can check whether ekat was
# already built, and avoid rebuilding it (which would yield a cmake error.
# You could do the same manually, but using this script you can keep
# some of the book-keeping logic out of your project, which you
# will need if some parts of your project (perhaps some submodules)
# are trying to use Ekat as well, and might try to build it internally.
# Additionally, the BuildEkat macro can allow you to inherit some config
# options from your projec, see comment below.

# Define global property for EKAT_BUILT to ensure ekat is built only once
define_property(GLOBAL
                PROPERTY EKAT_BUILT
                BRIEF_DOCS "Whether ekat subdir has already been processed"
                FULL_DOCS "This property is used by cmake to ensure that EKAT
                           submodule directory is only processed once (with add_subdirectory).")

get_property(IS_EKAT_BUILT GLOBAL PROPERTY EKAT_BUILT SET)

# This macro builds EKAT library (if not done already)
# The user can pass an additional variable PREFIX, which will be used
# to initialize all the EKAT_BLAH variable to the value stored in ${PREFIX}_BLAH.
# If ${PREFIX}_BLAH is not set, a default will be used.
# Furthermore, we give the user the ability to override ${PREFIX}_BLAH by
# explicitly passing BLAH blahVal in the macro call.
# E.g., consider this:
#
#  BuildEkat (PREFIX "MY_PROJECT" TEST_MAX_THREADS 1)
#
# It would build Ekat from the src tree found in ../../ekat (and error out
# if the path is wrong), set all the EKAT_XYZ var to match ${MY_PROJECT}_XYZ (or
# some predefined default, if ${MY_PROJECT}_XYZ is not set), but it would make
# sure that EKAT_TEST_MAX_THREADS is 1, regardless of what the corresponding
# MY_PROJECT_TEST_MAX_THREADS is
macro (BuildEkat)

  # Process EKAT only if not already done
  if (NOT IS_EKAT_BUILT)

    set(options)
    set(oneValueArgs
      PREFIX
      MPI_ERRORS_ARE_FATAL
      CONSTEXPR_ASSERT
      MIMIC_GPU
      # The following are only for testing
      ENABLE_TESTS
      TEST_MAX_THREADS
      TEST_THREADS_INC
      TEST_PACK_SIZE
      TEST_SMALL_PACK_SIZE
      TEST_POSSIBLY_NO_PACK
      TEST_POSSIBLY_NO_PACK_SIZE
      TEST_STRICT_FP
    )
    set(multiValueArgs)
    cmake_parse_arguments(BUILD_EKAT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    # First, set EKAT_BLAH to ${${PREFIX}_BLAH}. If not set, DO set defaults
    setVars("${BUILD_EKAT_PREFIX}" TRUE)

    # Then parse the optional input, and if set, override the existing value.
    # DO NOT set defaults, or you may override something
    setVars("BUILD_EKAT" FALSE)

    add_subdirectory (${EKAT_CMAKE_PATH}/../ ${CMAKE_BINARY_DIR}/externals/ekat)

    # Make sure that future includes of this script don't rebuild ekat
    set_property(GLOBAL PROPERTY EKAT_BUILT TRUE)
  else ()
    message ("Using Ekat previously configured in this project.\n")
  endif()
endmacro(BuildEkat)

# Set EKAT_BLAH variables from ${PREFIX}_BLAH variables. If the latter are not
# defined, use some defaults
# Note: all the EKAT variables MUST be cache variables, or EKAT will overwrite them
#       when calling set (EKAT_BLAH value TYPE CACHE "")
macro (setVars PREFIX SET_DEFAULTS)

  # Determine if this is a debug build
  string(TOLOWER "${CMAKE_BUILD_TYPE}" setVars_CMAKE_BUILD_TYPE_ci)
  if ("${setVars_CMAKE_BUILD_TYPE_ci}" STREQUAL "debug")
    set (setVars_DEBUG_BUILD TRUE)
  else ()
    set (setVars_DEBUG_BUILD FALSE)
  endif ()

  ### Needed to configure ekat ###

  if (DEFINED ${PREFIX}_MIMIC_GPU)
    set (EKAT_MIMIC_GPU ${${PREFIX}_MIMIC_GPU} CACHE BOOL "")
  endif()

  if (DEFINED ${PREFIX}_MPI_ERRORS_ARE_FATAL)
    set (EKAT_MPI_ERRORS_ARE_FATAL ${${PREFIX}_MPI_ERRORS_ARE_FATAL} CACHE BOOL "")
  endif()

  if (DEFINED ${PREFIX}_FPMODEL)
    set (EKAT_FPMODEL ${${PREFIX}_FPMODEL} CACHE STRING "")
  elseif (setVars_DEBUG_BUILD AND SET_DEFAULTS)
    set (EKAT_FPMODEL "strict" CACHE STRING "")
  endif()

  if (DEFINED ${PREFIX}_PACK_CHECK_BOUNDS)
    set (EKAT_PACK_CHECK_BOUNDS ${${PREFIX}_PACK_CHECK_BOUNDS} CACHE BOOL "")
  endif()

  if (DEFINED ${PREFIX}_DISABLE_TPL_WARNINGS)
    set (EKAT_DISABLE_TPL_WARNINGS ${${PREFIX}_DISABLE_TPL_WARNINGS} CACHE BOOL "")
  elseif (SET_DEFAULTS)
    set (EKAT_DISABLE_TPL_WARNINGS OFF CACHE BOOL "")
  endif()

  if (DEFINED ${PREFIX}_FPE)
    set (EKAT_FPE ${${PREFIX}_FPE} CACHE BOOL "")
  elseif (SET_DEFAULTS)
    set (EKAT_FPE OFF CACHE BOOL "")
  endif()

  if (DEFINED ${PREFIX}_ENABLE_TESTS)
    set (EKAT_ENABLE_TESTS ${${PREFIX}_ENABLE_TESTS} CACHE BOOL "")
  elseif (SET_DEFAULTS)
    set (EKAT_ENABLE_TESTS ON CACHE BOOL "")
  endif()

  ### Needed only if EKAT_ENABLE_TESTS=ON ###

  if (DEFINED ${PREFIX}_TEST_MAX_THREADS)
    set (EKAT_TEST_MAX_THREADS ${${PREFIX}_TEST_MAX_THREADS} CACHE STRING "")
  elseif (SET_DEFAULTS)
    set (EKAT_TEST_MAX_THREADS 1 CACHE STRING "")
  endif()

  if (DEFINED ${PREFIX}_TEST_THREAD_INC)
    set (EKAT_TEST_THREAD_INC ${${PREFIX}_TEST_THREAD_INC} CACHE STRING "")
  elseif (SET_DEFAULTS)
    set (EKAT_TEST_THREAD_INC 1 CACHE STRING "")
  endif()

  if (DEFINED ${PREFIX}_TEST_STRICT_FP)
    set (EKAT_TEST_STRICT_FP ${${PREFIX}_TEST_STRICT_FP} CACHE STRING "")
  elseif (setVars_DEBUG_BUILD AND SET_DEFAULTS)
    set (EKAT_TEST_STRICT_FP ON CACHE BOOL "")
  endif()

  if (DEFINED ${PREFIX}_TEST_PACK_SIZE)
    set (EKAT_TEST_PACK_SIZE ${${PREFIX}_TEST_PACK_SIZE} CACHE STRING "")
  elseif (SET_DEFAULTS)
    if (Kokkos_ENABLE_CUDA)
      set (EKAT_TEST_PACK_SIZE 1 CACHE STRING "")
    else ()
      set (EKAT_TEST_PACK_SIZE 16 CACHE STRING "")
    endif()
  endif()

  if (DEFINED ${PREFIX}_TEST_SMALL_PACK_SIZE)
    set (EKAT_TEST_SMALL_PACK_SIZE ${${PREFIX}_TEST_SMALL_PACK_SIZE} CACHE STRING "")
  elseif (SET_DEFAULTS)
    set (EKAT_TEST_SMALL_PACK_SIZE ${EKAT_TEST_PACK_SIZE} CACHE STRING "")
  endif()

  if (DEFINED ${PREFIX}_TEST_POSSIBLY_NO_PACK)
    set (EKAT_TEST_POSSIBLY_TEST_NO_PACK ${${PREFIX}_TEST_POSSIBLY_NO_PACK} CACHE STRING "")
  elseif (SET_DEFAULTS)
    if (Kokkos_ARCH_SKX)
      set (EKAT_TEST_POSSIBLY_TEST_NO_PACK TRUE)
    else ()
      set (EKAT_TEST_POSSIBLY_NO_PACK FALSE)
    endif ()
  endif()

  ### Cleanup ###

  unset (setVars_CMAKE_BUILD_TYPE_ci)
  unset (setVars_DEBUG_BUILD)
  unset (setVars_CUDA_POS)
endmacro (setVars)
