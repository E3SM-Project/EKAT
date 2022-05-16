# Detect the library that provides MPI
set (EKAT_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
macro (GetMpiDistributionName DISTRO_NAME)
  if (MPI_CXX_COMPILER)
    set (INCLUDE_DIRS ${MPI_CXX_INCLUDE_DIRS})
    set (SOURCE_FILE ${EKAT_CMAKE_DIR}/TryCompileMPI.cxx)
  elseif (MPI_C_COMPILER)
    set (INCLUDE_DIRS ${MPI_C_INCLUDE_DIRS})
    set (SOURCE_FILE ${EKAT_CMAKE_DIR}/TryCompileMPI.c)
  else ()
    string (CONCAT MSG
      "**************************************************************\n"
      "  MPI_C_COMPILER and MPI_CXX_COMPILER are not set.\n"
      "  CMake logic to determine the distribution name\n"
      "  requires a valid C or CXX mpi compiler.\n"
      "  Please call find_package(MPI [REQUIRED] COMPONENTS [C|CXX])\n"
      "  *before* calling GetMpiDistributionName (in the same scope).\n"
      "**************************************************************\n")
    message ("${MSG}")
    message (FATAL_ERROR "Aborting")
  endif()
  try_compile (RESULT
               ${CMAKE_BINARY_DIR}/CMakeFiles
               SOURCES ${SOURCE_FILE}
               CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${INCLUDE_DIRS}"
               OUTPUT_VARIABLE OUT_VAR)

  if (NOT RESULT)
    message (FATAL_ERROR "Could not compile a simple MPI source file.")
  endif()

  if (OUT_VAR MATCHES "OpenMPI")
    set (${DISTRO_NAME} "openmpi")
  elseif (OUT_VAR MATCHES "MPICH")
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
