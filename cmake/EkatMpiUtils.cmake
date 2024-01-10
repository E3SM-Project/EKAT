# Detect the library that provides MPI, by looking for vendor-specific
# macros inside the mpi.h header file
set (EKAT_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
macro (GetMpiDistributionName DISTRO_NAME)
  find_package (MPI REQUIRED QUIET)

  # We need the MPI headers folder, but we don't know which language is enabled
  # by the project which is calling this macro, so we try C/CXX/Fortran.
  if (CMAKE_C_COMPILER)
    set (MPI_H ${MPI_C_INCLUDE_DIRS}/mpi.h)
  elseif (CMAKE_CXX_COMPILER)
    set (MPI_H ${MPI_CXX_INCLUDE_DIRS}/mpi.h)
  elseif (CMAKE_Fortran_COMPILER)
    set (MPI_H ${MPI_Fortran_F77_HEADER_DIR}/mpi.h)
  else ()
    string (CONCAT MSG
      "**************************************************************\n"
      "  CMake logic to determine the distribution name\n"
      "  requires a valid C, CXX, or Fortran compiler.\n"
      "**************************************************************\n")
    message ("${MSG}")
    message (FATAL_ERROR "Aborting")
  endif()

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
