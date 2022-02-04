include (EkatUtils)

# This macro adds ekat's valgrind_support subfolder, which generates
# valgrind suppression files for both serial and MPI calls

# The following optional arguments can be passed
#  - MPIEXEC: command to be used to run MPI (requires mpicxx as the CXX compiler)
#  - SUPP_FILES_DIR: where to put the generated suppression files.
#    Defaults to ${CMAKE_BINARY_DIR}

# Note: we have to set this variable here, so CMAKE_CURRENT_LIST_DIR gets the
#       directory of this file. If we did it inside the function, it would get
#       the directory from where the function is called
set (EKAT_VALG_SUPP_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../valgrind_support)

# Define global property for valgrind support, to ensure it's built only once
define_property(GLOBAL
                PROPERTY EKAT_VALG_SUPPORT_BUILT
                BRIEF_DOCS "Whether ekat valgrind support subdir has already been processed"
                FULL_DOCS "This property is used by cmake to ensure that EKAT
                           valgrind support directory is only processed once (with add_subdirectory).")

get_property(IS_VALG_SUPPORT_BUILT GLOBAL PROPERTY EKAT_VALG_SUPPORT_BUILT SET)

function (EkatGenerateValgrindSuppressions)

  if (NOT IS_VALG_SUPPORT_BUILT)
    # EGVS = Ekat Generate Valgrind Suppressions
    set(EGVS_OPTIONS)
    set(EGVS_1V_ARGS MPIEXEC SUPP_FILES_DIR)
    set(EGVS_MV_ARGS)

    cmake_parse_arguments(egvs "${EGVS_OPTIONS}" "${EGVS_1V_ARGS}" "${EGVS_MV_ARGS}" ${ARGN})
    CheckMacroArgs(EkatGenerateValgrindSuppressions egvs "${EGVS_OPTIONS}" "${EGVS_1V_ARGS}" "${EGVS_MV_ARGS}")

    add_subdirectory (${EKAT_VALG_SUPP_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/valgrind_support)

    set_property(GLOBAL PROPERTY EKAT_VALG_SUPPORT_BUILT TRUE)
  else()
    message("ALREADY BUILT!")
  endif()
endfunction()
