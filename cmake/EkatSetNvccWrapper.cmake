# Note: We cannot use 'EKAT_SOURCE_DIR', since one may be calling this macro
#       before processing the ekat subfolder. We have to compute the location
#       of ekat_mpicxx.in relative to this file.
set (MPICXX_WRAPPER_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../bin)
macro (EkatSetNvccWrapper)
  # Check if ekat_mpicxx is already the CMAKE_CXX_COMPILER. This could happen if
  # one configures the project, then changes something that triggers cmake to run.
  # If that happens, without this if guard, the value of MPICXX in the ekat_mpicxx
  # script would be the ekat_mpicxx script itself, causing infinite recursion. 
  if (NOT "${CMAKE_CXX_COMPILER}" STREQUAL "${CMAKE_BINARY_DIR}/bin/ekat_mpicxx")
    SetMpiCxxBackendCompilerVarName("MPI_CXX_BACKEND_COMPILER_VAR_NAME")
    # Before starting the project, wrap mpicxx in the ekat_mpicxx script, which
    # takes care of setting OMPI_CXX to point to nvcc_wrapper

    if (NOT KOKKOS_SOURCE_DIR)
      message (FATAL_ERROR "Error! When calling 'EkatSetNvccWrapper', KOKKOS_SOURCE_DIR must be already set.")
    endif ()

    configure_file(${MPICXX_WRAPPER_SOURCE_DIR}/ekat_mpicxx.in ${CMAKE_BINARY_DIR}/bin/ekat_mpicxx @ONLY)
    set(CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/bin/ekat_mpicxx CACHE STRING "" FORCE)
  endif ()
endmacro()
