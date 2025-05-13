include(GNUInstallDirs)

# spdlog is a REQUIRED dep of this package

option (EKAT_SKIP_FIND_SPDLOG "Skip find_package for spdlog, and  build directly from submodule" OFF)
if (EKAT_SKIP_FIND_SPDLOG)
  # The user does not want to accidentally get an existing installation,
  # and instead wants to force a build from the internal submodule
  include (EkatBuildSpdlog)
else()
  # We first try to use find_XYZ. If that doesn't work, build from submodule
  message (STATUS "Looking for spdlog ...")
  find_package(spdlog QUIET)
  if (NOT spdlog_FOUND)
    message (STATUS "Looking for spdlog ... NOT FOUND")
    include(EkatBuildSpdlog)
  else()
    message (STATUS "Looking for spdlog ... FOUND")
    message (STATUS "  spdlog_DIR: ${spdlog_DIR}")
  endif()
endif()

# Create the library, and set all its properties
# NOTE: since there are NO .cpp files, we must declare the target as INTERFACE
add_library(Logging INTERFACE)
target_link_libraries(Logging PUBLIC Core)
target_link_libraries(ekat PUBLIC spdlog::spdlog)

target_include_directories(Logging PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)

set (HEADERS
  ekat_log_file_policy.hpp
  ekat_log_mpi_policy.hpp
  ekat_logger.hpp
)

# Set the PUBLIC_HEADER property
set_target_properties(Logging PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS Logging
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)
