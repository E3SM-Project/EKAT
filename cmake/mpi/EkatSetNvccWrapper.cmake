# Note: We cannot use 'EKAT_SOURCE_DIR', since one may be calling this macro
#       before processing the ekat subfolder. We have to compute the location
#       of ekat_mpicxx.in relative to this file.
set (MPICXX_WRAPPER_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../bin)
macro (EkatSetNvccWrapper)
  # One filename full path could contain symlinks while the other doesn't.
  # This would cause the two string to compare different, while the actual
  # files would be the same. So get the *real* files full paths first.
  get_filename_component(cxx_compiler "${CMAKE_CXX_COMPILER}" REALPATH)
  get_filename_component(ekat_mpicxx "${CMAKE_BINARY_DIR}/bin/ekat_mpicxx" REALPATH)

  # Check if ekat_mpicxx is already the CMAKE_CXX_COMPILER. This could happen if
  # one configures the project, then changes something that triggers cmake to run.
  # If that happens, without this if guard, we would generate an ekat_mpicxx
  # where the value of MPICXX would ekat_mpicxx itself, causing infinite recursion
  # every time the compiler is invoked.
  if (NOT cxx_compiler STREQUAL ekat_mpicxx)
    SetMpiCxxBackendCompilerVarName("MPI_CXX_BACKEND_COMPILER_VAR_NAME")
    # Before starting the project, wrap mpicxx in the ekat_mpicxx script, which
    # takes care of setting OMPI_CXX to point to nvcc_wrapper

    if (NOT Kokkos_SOURCE_DIR)
      message (FATAL_ERROR "Error! When calling 'EkatSetNvccWrapper', Kokkos_SOURCE_DIR must be already set.")
    endif ()

    configure_file(${MPICXX_WRAPPER_SOURCE_DIR}/ekat_mpicxx.in ${CMAKE_BINARY_DIR}/bin/ekat_mpicxx @ONLY)
    set(CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/bin/ekat_mpicxx CACHE STRING "" FORCE)

    # Install ekat_mpicxx for downstream apps to use
    include(GNUInstallDirs)
    install (PROGRAMS ${CMAKE_BINARY_DIR}/bin/ekat_mpicxx
             DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif ()
endmacro()
