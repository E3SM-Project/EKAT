include(GNUInstallDirs)

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

    # Parse optional inputs. Notice that we put boolean options as single-value args,
    # since we want the user to be able to explicitly set them to OFF, rather than
    # let EKAT pick. Besides, it's much easier to do
    #   BuildEkat(arg1 value1 arg7 value2 ...)
    # rather than building the string "arg1 arg7 ..." based on host project config
    set(options)
    set(boolArgs
      ENABLE_VALGRIND
      ENABLE_CUDA_MEMCHECK
      ENABLE_COVERAGE
      ENABLE_FPE
      ENABLE_DEFAULT_FPE
      MPI_ERRORS_ARE_FATAL
      CONSTEXPR_ASSERT
      DISABLE_TPL_WARNINGS
      DEFAULT_BFB
      MIMIC_GPU
      # The following are only for EKAT's testing
      ENABLE_TESTS
      TEST_STRICT_FP
      TEST_POSSIBLY_NO_PACK
    )
    set(strArgs
      # The following are only for EKAT's testing
      TEST_MAX_THREADS
      TEST_THREADS_INC
      TEST_PACK_SIZE
      TEST_SMALL_PACK_SIZE
      TEST_POSSIBLY_NO_PACK_SIZE
    )
    set(multiValueArgs)
    cmake_parse_arguments(be "${options}" "${boolArgs} ${strArgs}" "${multiValueArgs}" ${ARGN} )

    # Parse the optional arguments. If set, use them to set cache vars.
    foreach (opt IN ITEMS ${boolArgs})
      if (DEFINED be_${opt})
        set (EKAT_${opt} ${be_${opt}} CACHE BOOL "")
      endif()
    endforeach()
    foreach (arg IN ITEMS ${strArgs})
      if (DEFINED be_${arg})
        set (EKAT_${arg} ${be_${arg}} CACHE STRING "")
      endif()
    endforeach()

    # Now that all user-requested ekat options are set, process ekat subdir.
    # NOTE: ekat's CMakeLists.txt will pick defaults for all unset ekat config options
    add_subdirectory (${EKAT_CMAKE_PATH}/../ ${CMAKE_BINARY_DIR}/externals/ekat)

    if (EKAT_DISABLE_TPL_WARNINGS)
      include (EkatUtils)
      EkatDisableAllWarning(ekat)
      EkatDisableAllWarning(ekat_test_main)
      EkatDisableAllWarning(ekat_test_session)
    endif ()

    # Make sure that future includes of this script don't rebuild ekat
    set_property(GLOBAL PROPERTY EKAT_BUILT TRUE)
  else ()
    message ("Using Ekat previously configured in this project.\n")
  endif()
endmacro(BuildEkat)
