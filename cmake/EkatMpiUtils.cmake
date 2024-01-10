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
  find_file (MPI_H mpi.h
    PATHS ${MPI_C_INCLUDE_DIRS}
          ${MPI_C_INCLUDE_PATH}
          ${MPI_CXX_INCLUDE_DIRS}
          ${MPI_CXX_INCLUDE_PATH}
          ${MPI_Fortran_F77_HEADER_DIR}
          ${MPI_INCLUDE_PATH}
          ${MPI_INC_DIR})

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
