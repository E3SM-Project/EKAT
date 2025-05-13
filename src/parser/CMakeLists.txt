# yaml-cpp is a REQUIRED dep of this package

option (EKAT_SKIP_FIND_YAML_CPP
  "Skip find_package for yaml-cpp, and  build directly from submodule" OFF)
if (EKAT_SKIP_FIND_YAML_CPP)
  # The user does not want to accidentally get an existing installation,
  # and instead wants to force a build from the internal submodule
  include (EkatBuildYamlCpp)
else()
  # I am having issues getting the env var YAML_CPP_ROOT being picked up
  # by cmake. I suspect this has to do with the presence of the hyphen
  # CMake *should* convert '-' to '_' when forming the cmake/env var name,
  # so YAML_CPP_ROOT should be recognized. Alas, it's not on my machine,
  # running CMake 3.26.3, so we must pass the var explicitly via HINTS
  message (STATUS "Looking for yaml-cpp ...")
  if (NOT YAML_CPP_ROOT AND NOT "$ENV{YAML_CPP_ROOT}" STREQUAL "")
    set (YAML_CPP_ROOT $ENV{YAML_CPP_ROOT})
  endif()

  find_package(yaml-cpp HINTS ${YAML_CPP_ROOT})
  if (NOT yaml-cpp_FOUND)
    message (STATUS "Looking for yaml-cpp ... NOT FOUND")
    include(EkatBuildYamlCpp)
  else()
    message (STATUS "Looking for yaml-cpp ... FOUND")
    message (STATUS "  yaml-cpp_DIR: ${yaml-cpp_DIR}")
    # It is possible that the installation provides the yaml-cpp target, but not the yaml-cpp::yaml-cpp target. If so, define the alias target
    if (NOT TARGET yaml-cpp::yaml-cpp)
      if (NOT TARGET yaml-cpp)
        message (FATAL_ERROR "Neither yaml-cpp nor yaml-cpp::yaml-cpp targets were found")
      endif()
      add_library (yaml-cpp::yaml-cpp ALIAS yaml-cpp)
    endif()
  endif()
endif()

# Create the library, and set all its properties
add_library(YamlParser
  ekat_yaml.cpp
)
target_link_libraries(YamlParser PUBLIC Core)
target_link_libraries(YamlParser PRIVATE yaml-cpp::yaml-cpp)

target_include_directories(YamlParser INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)


# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_yaml.hpp
)
set_target_properties (YamlParser PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS YamlParser
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)
