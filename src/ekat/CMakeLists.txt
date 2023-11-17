include(GNUInstallDirs)
include(EkatSetCompilerFlags)
include (EkatUtils)

# Generate configuration files
if (EKAT_ENABLE_FORTRAN)
  EkatConfigFile (${CMAKE_CURRENT_SOURCE_DIR}/ekat_config.h.in
                  ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.h
                  F90_FILE ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.f)
else()
  EkatConfigFile (${CMAKE_CURRENT_SOURCE_DIR}/ekat_config.h.in
                  ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.h)
endif()

set(EKAT_SOURCES
  ekat_assert.cpp
  ekat_parameter_list.cpp
  ekat_session.cpp
  io/ekat_array_io.cpp
  util/ekat_arch.cpp
  util/ekat_string_utils.cpp
  util/ekat_test_utils.cpp
)
if (EKAT_ENABLE_YAML_PARSER)
  list (APPEND EKAT_SOURCES io/ekat_yaml.cpp)
endif()
if (EKAT_ENABLE_FORTRAN)
  set(EKAT_SOURCES ${EKAT_SOURCES} io/ekat_array_io_mod.f90)
endif()

if (EKAT_ENABLE_MPI)
  set(EKAT_SOURCES ${EKAT_SOURCES} mpi/ekat_comm.cpp)
else()
  set(EKAT_SOURCES ${EKAT_SOURCES} mpi/ekat_comm_serial.cpp)
endif()

# Create the library, and set all its properties
add_library(ekat ${EKAT_SOURCES})

# These libs are for sure needed
target_link_libraries(ekat PUBLIC Kokkos::kokkos spdlog::spdlog)

# These libs are optional
if (EKAT_ENABLE_MPI)
  target_link_libraries(ekat PUBLIC MPI::MPI_C)
endif()
if (EKAT_ENABLE_YAML_PARSER)
  target_link_libraries(ekat PRIVATE yaml-cpp::yaml-cpp)
endif()
if (EKAT_HAS_STACKTRACE)
  target_link_libraries(ekat PUBLIC ${Boost_LIBRARIES})
endif()



target_include_directories(ekat PUBLIC
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${EKAT_BINARY_DIR}/src>
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/extern/Catch2/single_include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
if (EKAT_ENABLE_FORTRAN)
  target_include_directories(ekat PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/ekat_f90_modules>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat_f90_modules>)
  # Enable C Pre-Processor for Fortran.
  # No reason to pollute C/C++ flags (CPP always runs), so add the flag only for Fortran files.
  target_compile_options(ekat PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-cpp>)
  set_target_properties(ekat PROPERTIES
      Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ekat_f90_modules)
endif()

# TODO: The current version of kokkos we are using in e3sm has a nvcc_wrapper
#       which breaks the cmake toolchain features detection mechanisms.
#       If we update kokkos to, say, current develop (as of 06/2020),
#       the following line would work fine.
# target_compile_features(ekat PUBLIC cxx_std_11)

if (EKAT_HAVE_FEENABLEEXCEPT)
  # This is needed internally by ekat, so let's keep it private
  target_compile_definitions (ekat PRIVATE EKAT_HAVE_FEENABLEEXCEPT)
endif()

install (TARGETS ekat
         EXPORT EkatTargets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Unfortunately, adding PUBLIC_HEADER to the install above would flatten the directory
# structure, which would cause include errors (in ekat we keep the directory
# tree in the inlcude, like "ekat/mpi/ekat_comm.hpp")
# Therefore, manually install the src tree (filter only header files), as well
# as the config files in the bin dir
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         FILES_MATCHING PATTERN *.hpp)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.h
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)
if (EKAT_ENABLE_FORTRAN)
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.f
           DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)
  install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ekat_f90_modules
           DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

###########################################
###       Testing micro-libraries       ###
###########################################

# Pre-compile ekat_catch_main.cpp into its own mini-library, so that individual
# unit test don't have to all pointlessly rebuild the same file.
add_library(ekat_test_main util/ekat_catch_main.cpp)
target_link_libraries(ekat_test_main PUBLIC ekat)
target_include_directories(ekat_test_main PUBLIC
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/extern/Catch2/single_include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)

install (TARGETS ekat_test_main
         EXPORT EkatTargets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install (DIRECTORY ${EKAT_SOURCE_DIR}/extern/Catch2/single_include/catch2
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Pre-compile ekat_test_session.cpp into its own mini-library.
# This mini-library has to be linked if you are ok relying on Ekat's test session
# initialization/finalization routines (which simply call initialize_ekat_session
# and finalize_ekat_session respectively). If your test needs to do more stuff
# at init/finalize time, then you should NOT link this mini library, and provide
# init/finalize routines in a separate source file (to be linked in).
# unit test don't have to all pointlessly rebuild the same file.
add_library(ekat_test_session util/ekat_test_session.cpp)
target_link_libraries(ekat_test_session PUBLIC ekat)

install (TARGETS ekat_test_session
         EXPORT EkatTargets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
