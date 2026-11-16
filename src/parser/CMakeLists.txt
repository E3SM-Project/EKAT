# yaml-cpp is a REQUIRED dep of this package

set (yaml-cpp_FOUND FALSE)
if (TARGET yaml-cpp::yaml-cpp OR TARGET yaml-cpp)
  set(yaml-cpp_FOUND TRUE)
  message(STATUS "EKAT: Using yaml-cpp provided by parent project")
endif()

option (EKAT_SKIP_FIND_YAML_CPP
  "Skip find_package for yaml-cpp, and fetch via FetchContent" OFF)
if (NOT yaml-cpp_FOUND AND NOT EKAT_SKIP_FIND_YAML_CPP)
  # cmake builds the name of the root var/env var from the package name, so for yaml-cpp
  # we would need yaml-cpp_ROOT. Since env vars with a hyphen in their name are awkward
  # to set in a shell, we also support YAML_CPP_ROOT (both as cmake var and as env var).
  if (NOT yaml-cpp_ROOT)
    if (YAML_CPP_ROOT)
      set (yaml-cpp_ROOT ${YAML_CPP_ROOT})
    elseif (NOT "$ENV{YAML_CPP_ROOT}" STREQUAL "")
      set (yaml-cpp_ROOT $ENV{YAML_CPP_ROOT})
    endif()
  endif()

  include (EkatFindTpl)
  ekat_find_tpl(yaml-cpp)
  if (yaml-cpp_FOUND)
    message (STATUS "Looking for yaml-cpp ... FOUND")
    message (STATUS "  yaml-cpp_DIR: ${yaml-cpp_DIR}")
  else()
    message (STATUS "Looking for yaml-cpp ... NOT FOUND")
  endif()
endif()

# If we found yaml-cpp (via find_package or as an existing tgt)
# ensure it provides the scoped target
if (yaml-cpp_FOUND)
  if (TARGET yaml-cpp AND NOT TARGET yaml-cpp::yaml-cpp)
    add_library(yaml-cpp::yaml-cpp ALIAS yaml-cpp)
  endif()
endif()

if (NOT yaml-cpp_FOUND)
  include(EkatBuildYamlCpp)
  set (EKAT_BUILDS_YAMLCPP ON CACHE BOOL "Whether Ekat builds yaml-cpp" FORCE)
else()
  set (EKAT_BUILDS_YAMLCPP OFF CACHE BOOL "Whether Ekat builds yaml-cpp" FORCE)
endif()

# Create the library, and set all its properties
add_library(ekat_yamlparser
  ekat_yaml.cpp
)
target_link_libraries(ekat_yamlparser PUBLIC ekat::Core)
target_link_libraries(ekat_yamlparser PRIVATE yaml-cpp::yaml-cpp)

target_include_directories(ekat_yamlparser INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)


# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_yaml.hpp
)
set_target_properties (ekat_yamlparser PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::YamlParser
set_target_properties(ekat_yamlparser PROPERTIES
  EXPORT_NAME YamlParser
  PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS ekat_yamlparser
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::YamlParser ALIAS ekat_yamlparser)

# Link to the all libs target
target_link_libraries(ekat_all_libs INTERFACE ekat_yamlparser)
