include(GNUInstallDirs)

# spdlog is a REQUIRED dep of this package

option (EKAT_SKIP_FIND_SPDLOG "Skip find_package for spdlog, and fetch via FetchContent" OFF)
if (NOT EKAT_SKIP_FIND_SPDLOG)
  message (STATUS "Looking for spdlog ...")
  find_package(spdlog QUIET NO_DEFAULT_PATH)
  if (spdlog_FOUND)
    message (STATUS "Looking for spdlog ... FOUND")
    message (STATUS "  spdlog_DIR: ${spdlog_DIR}")
    # It is possible that the installation provides the spdlog target, but not the spdlog::spdlog target. If so, define the alias target
    if (NOT TARGET spdlog::spdlog)
      if (NOT TARGET spdlog)
        message (FATAL_ERROR "Neither spdlog nor spdlog::spdlog targets were found")
      endif()
      add_library (spdlog::spdlog ALIAS spdlog)
    endif()
  else()
    message (STATUS "Looking for spdlog ... NOT FOUND")
  endif()
else ()
  set (spdlog_FOUND FALSE)
endif()

if (NOT spdlog_FOUND)
  include(EkatBuildSpdlog)
  set (EKAT_BUILDS_SPDLOG ON CACHE BOOL "Whether Ekat builds spdlog" FORCE)
else()
  set (EKAT_BUILDS_SPDLOG OFF CACHE BOOL "Whether Ekat builds spdlog" FORCE)
endif()

# Create the library, and set all its properties
# NOTE: since there are NO .cpp files, we must declare the target as INTERFACE
add_library(ekat_logging INTERFACE)
target_link_libraries(ekat_logging INTERFACE ekat::Core)
target_link_libraries(ekat_logging INTERFACE spdlog::spdlog)

target_include_directories(ekat_logging INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)

# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_log_file_policy.hpp
  ekat_log_mpi_policy.hpp
  ekat_logger.hpp
)
set_target_properties(ekat_logging PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::ekat_logging
set_target_properties(ekat_logging PROPERTIES
  EXPORT_NAME Logging
  PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS ekat_logging
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::Logging ALIAS ekat_logging)

# Link to the all libs target
target_link_libraries(ekat_all_libs INTERFACE ekat_logging)
