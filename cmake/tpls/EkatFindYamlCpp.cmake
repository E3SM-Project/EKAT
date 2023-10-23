# NOTE: unfortunately yaml-cpp does not offer a CMake installation,
#       so we need to find libs/headers by hand
message (STATUS "Looking for yaml ...")
if (NOT "$ENV{YAML_CPP_ROOT}" STREQUAL "")
  set (YAML_CPP_ROOT $ENV{YAML_CPP_ROOT})
endif()

if (YAML_CPP_ROOT)
  # Look for headers and lib
  find_path(YAML_CPP_INCLUDE_DIR
    NAMES yaml.h
    PATHS ${YAML_CPP_ROOT}
    PATH_SUFFIXES include include/yaml-cpp)
  find_library(YAML_CPP_LIB
    NAMES yaml-cpp yaml-cppd
    PATHS ${YAML_CPP_ROOT}
    PATH_SUFFIXES lib lib64)
  if (NOT YAML_CPP_LIB STREQUAL "YAML_CPP_LIB-NOTFOUND" AND
      NOT YAML_CPP_INCLUDE_DIR STREQUAL "YAML_CPP_INCLUDE_DIR-NOTFOUND")
    set (YAML_CPP_FOUND TRUE)
  endif()
endif()

if (YAML_CPP_FOUND)
  message (STATUS "Looking for yaml-cpp... FOUND")
  message (STATUS "  YAML_CPP_LIB: ${YAML_CPP_LIB}")
  message (STATUS "  YAML_CPP_INCLUDE DIR: ${YAML_CPP_INCLUDE_DIR}")
  add_library(yaml-cpp INTERFACE)
  target_link_libraries (yaml-cpp INTERFACE ${YAML_CPP_LIB})
  target_include_directories (yaml-cpp INTERFACE ${YAML_CPP_INCLUDE_DIR})
else()
  message (STATUS "Looking for yaml-cpp... NOT FOUND")
endif()

