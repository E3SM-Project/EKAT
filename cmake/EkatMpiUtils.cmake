# Detect the library that provides MPI, by looking for vendor-specific
# macros inside the mpi.h header file
set (EKAT_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
macro (GetMpiDistributionName DISTRO_NAME)

  # If find_package(MPI [...]) was already called, this is a no op.
  find_package (MPI REQUIRED QUIET)

  # We need the MPI headers folder, but we don't know which language is enabled
  # by the project which is calling this macro, which means we don't know which
  # of the MPI_XYZ cmake vars were populated by the above call. So add all the vars
  # that can possibly be filled by FindMPI in different versions of cmake.
  # Some will be empty, some will be redundant, but that's ok.
  set (MPI_H_PATHS)
  list(APPEND MPI_H_PATHS
    ${MPI_C_INCLUDE_DIRS}
    ${MPI_C_INCLUDE_PATH}
    ${MPI_CXX_INCLUDE_DIRS}
    ${MPI_CXX_INCLUDE_PATH}
    ${MPI_Fortran_F77_HEADER_DIR}
    ${MPI_INCLUDE_PATH}
    ${MPI_INC_DIR})

  # Sometimes all the above vars are empty, and we only have MPI_<LANG>_COMPILER set.
  # We can use the mpi compiler with the -show flag to get a list of the flags passed
  # to the backend compiler, which includes the include paths.
  if (MPI_CXX_COMPILER OR MPI_C_COMPILER OR MPI_Fortran_COMPILER)
    if (MPI_CXX_COMPILER)
      set (COMPILER ${MPI_CXX_COMPILER})
    elseif (MPI_C_COMPILER)
      set (COMPILER ${MPI_C_COMPILER})
    else()
      set (COMPILER ${MPI_Fortran_COMPILER})
    endif()

    execute_process (COMMAND ${COMPILER} -show RESULT_VARIABLE SUPPORTS_SHOW OUTPUT_QUIET ERROR_QUIET)
    execute_process (COMMAND ${COMPILER} --cray-print-opts=cflags RESULT_VARIABLE SUPPORTS_CRAY_PRINT_OPTS OUTPUT_QUIET ERROR_QUIET)
    if (SUPPORTS_SHOW EQUAL 0)
      # (mpicxx/mpicc/mpifort)-like MPI compiler
      execute_process (COMMAND ${COMPILER} -show OUTPUT_VARIABLE TEMP)
    elseif (SUPPORTS_CRAY_PRINT_OPTS EQUAL 0)
      # craype-like MPI wrapper compiler
      # Cray wrappers have different options from mpicxx
      execute_process (COMMAND ${COMPILER} --cray-print-opts=cflags OUTPUT_VARIABLE TEMP)
    else()
      # unknow MPI compiler
      string (CONCAT msgs
        " ** Unhandled scenario in ekat's GetMpiDistributionName **"
        "The MPI compiler does not support any known flag to show compile command\n"
        " - compiler: ${COMPILER}\n"
        " - known flags: '-show' (for mpicxx/mpicc/mpifort) '--cray-print-opts=cflags' (for cray's CC/cc/ftn)\n"
        "We will not be able to check the include paths of the MPI compiler when looking for mpi.h"
      )

      message (WARNING "${msg}")
    endif()

    # Remove spaces/commas, and parse each entry. If it starts with -I, it may be
    # an include path with mpi.h
    string(REPLACE " " ";" TEMP_LIST "${TEMP}")
    string(REPLACE "," ";" TEMP_LIST "${TEMP_LIST}")
    foreach (entry IN_ITEMS ${TEMP_LIST})
      if (entry MATCHES "^-I")
        string(REGEX REPLACE "-I([^ ]*)" "\\1"  this_path ${entry})
        list(APPEND MPI_H_PATHS ${this_path})
      endif()
    endforeach()
  endif()

  # Look for mpi.h
  find_file (MPI_H mpi.h
    PATHS ${MPI_H_PATHS})

  # Check what macros are defined in mpi.h
  include (CheckSymbolExists)
  check_symbol_exists(OMPI_MAJOR_VERSION ${MPI_H} HAVE_OMPI)
  check_symbol_exists(MPICH_VERSION ${MPI_H} HAVE_MPICH)

  if (HAVE_OMPI)
    set (${DISTRO_NAME} "openmpi")
  elseif (HAVE_MPICH)
    set (${DISTRO_NAME} "mpich")
  else ()
    set (${DISTRO_NAME} "unknown")
  endif()
endmacro ()

# Disable MPI cxx binding
function (DisableMpiCxxBindings)
  # OpenMPI disable inclusion of mpicxx.h if the OMPI_SKIP_MPICXX macro
  # is defined, while MPICH checks the MPICH_SKIP_MPICXX macro.
  # To keep things tidy and avoid defining pointless macros, we first
  # check the MPI distribution, in order to decide what to define

  set (DISTRO_NAME)
  GetMpiDistributionName (DISTRO_NAME)

  if ("${DISTRO_NAME}" STREQUAL "openmpi")
    add_definitions (-DOMPI_SKIP_MPICXX)
  elseif ("${DISTRO_NAME}" STREQUAL "mpich")
    add_definitions (-DMPICH_SKIP_MPICXX)
  else ()
    message (FATAL_ERROR "Unsupported MPI distribution.")
  endif()

  unset (DISTRO_NAME)
endfunction()

# Set MPI runtime env vars for comm world rank/size, depending on mpi distribution
# NOTE: this is needed by the test-launcher script you find in ${CMAKE_SOURCE_DIR}/bin
macro (SetMpiRuntimeEnvVars)
  set (DISTRO_NAME)
  GetMpiDistributionName (DISTRO_NAME)

  if (DISTRO_NAME STREQUAL "openmpi")
    set(EKAT_MPIRUN_COMM_WORLD_SIZE "OMPI_COMM_WORLD_SIZE")
    set(EKAT_MPIRUN_COMM_WORLD_RANK "OMPI_COMM_WORLD_RANK")
  elseif (DISTRO_NAME STREQUAL "mpich")
    set(EKAT_MPIRUN_COMM_WORLD_SIZE "PMI_SIZE")
    set(EKAT_MPIRUN_COMM_WORLD_RANK "PMI_RANK")
  else ()
    message (FATAL_ERROR "Unsupported MPI distribution.")
  endif()

  unset (DISTRO_NAME)
endmacro()
