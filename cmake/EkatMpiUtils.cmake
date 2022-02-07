# Detect the library that provides MPI
set (EKAT_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
macro (GetMpiDistributionName DISTRO_NAME)
  execute_process(COMMAND ${MPIEXEC_EXECUTABLE} --version
                  OUTPUT_VARIABLE OUT_VAR
                  RESULT_VARIABLE SUCCESS)
  if (NOT SUCCESS EQUAL 0)
    message (FATAL_ERROR
      "Error! Could not run '${MPIEXEC_EXECUTABLE} --version'")
  endif()

  if (OUT_VAR MATCHES "open-mpi|OpenMPI|Open MPI|OpenRTE")
    set (${DISTRO_NAME} "openmpi")
  elseif (OUT_VAR MATCHES "mpich|MPICH")
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
