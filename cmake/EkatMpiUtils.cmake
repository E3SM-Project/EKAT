# Detect the library that provides MPI
set (EKAT_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
macro (GetMpiDistributionName DISTRO_NAME)
  if (MPI_C_COMPILER)
    set (COMPILER_NAME ${MPI_C_COMPILER})
  elseif (MPI_CXX_COMPILER)
    set (COMPILER_NAME ${MPI_CXX_COMPILER})
  else ()
    string (CONCAT MSG
      "MPI_C_COMPILER and MPI_CXX_COMPILER are not set.\b"
      "CMake logic to determine the distribution name requires C/CXX mpi compilers.\n"
      "Please call find_package(MPI COMPONENTS [C|CXX]) *before* calling GetMpiDistributionName (and in the same scope)")
    message ("${MSG}")
    message (FATAL_ERROR "Aborting")
  endif()
  execute_process(COMMAND ${COMPILER_NAME} -c ${EKAT_CMAKE_DIR}/TryCompileMPI.cxx
                  OUTPUT_VARIABLE OUT_VAR
                  ERROR_VARIABLE OUT_VAR
                  RESULT_VARIABLE SUCCESS
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  if (OUT_VAR MATCHES "OpenMPI")
    set (${DISTRO_NAME} "openmpi")
  elseif (OUT_VAR MATCHES "MPICH")
    set (${DISTRO_NAME} "mpich")
  else ()
    set (${DISTRO_NAME} "unknown")
  endif()

  # Remove compiled file from src tree
  execute_process(COMMAND rm -f ${CMAKE_BINARY_DIR}/TryCompileMPI.cxx.o
                  OUTPUT_QUIET ERROR_QUIET
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
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
