include(GNUInstallDirs)

# MPI is enabled by default, but not needed.
option (EKAT_ENABLE_MPI "Whether EKAT requires MPI." ON)
if (EKAT_ENABLE_MPI)
  find_package(MPI REQUIRED)

  # NOTE: may be an overkill, but depending on which FindMPI module is called,
  #       if _FIND_REQUIRED is not checked, we may not get a fatal error
  #       if the required components are not found. So check the _FOUND var.
  if (NOT MPI_C_FOUND)
    message (FATAL_ERROR "EKAT *requires* the C component of MPI to be found")
  endif()

  # We should avoid cxx bindings in mpi; they are already deprecated,
  # and can cause headaches at link time, cause they require -lmpi_cxx
  # (openpmi) or -lmpicxx (mpich) flags.
  include(EkatMpiUtils)
  DisableMpiCxxBindings()

  # MPI-related options
  option (EKAT_MPI_ERRORS_ARE_FATAL " Whether EKAT should crash when MPI errors happen." ON)

  set(EKAT_MPIRUN_EXE "mpiexec" CACHE STRING "The executable name for mpirun")
  set(EKAT_MPI_EXTRA_ARGS "--bind-to core" CACHE STRING "Options for mpirun")
  set(EKAT_MPI_NP_FLAG "--map-by" CACHE STRING "The mpirun flag for designating the total number of ranks")
  set(EKAT_MPI_THREAD_FLAG "" CACHE STRING "The mpirun flag for designating the number of threads")
endif()

# Create the library, and set all its properties
if (EKAT_ENABLE_MPI)
  add_library(MpiComm
    ekat_comm.cpp
  )
  target_compile_definitions(MpiComm PUBLIC EKAT_ENABLE_MPI)
  target_link_libraries(MpiComm PUBLIC MPI::MPI_C)
else()
  add_library(MpiComm
    ekat_comm_serial.cpp
  )
endif()

if (EKAT_MPI_ERRORS_ARE_FATAL)
  target_compile_definitions(MpiComm PUBLIC EKAT_MPI_ERRORS_ARE_FATAL)
endif()

target_link_libraries(MpiComm PUBLIC Core)

target_include_directories(MpiComm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_comm.hpp
)
set_target_properties(MpiComm PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS MpiComm
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
