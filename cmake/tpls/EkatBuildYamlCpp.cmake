# NOTE: unfortunately yaml-cpp does not offer a CMake installation,
#       so we need to find libs/headers by hand
if (NOT TARGET yaml-cpp)
  message (STATUS "No yaml-cpp target already defined. Building locally from submodule")

  # Set options for yamlcpp before adding the subdirectory
  option (YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
  option (YAML_CPP_BUILD_TESTS "Enable yaml-cpp tests" OFF)

  set (YAML_CPP_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../extern/yaml-cpp)
  set (YAML_CPP_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/yaml-cpp)
  message (STATUS "  YAML_CPP_SOURCE_DIR: ${YAML_CPP_SOURCE_DIR}")
  message (STATUS "  YAML_CPP_BINARY_DIR: ${YAML_CPP_BINARY_DIR}")

  add_subdirectory (${YAML_CPP_SOURCE_DIR} ${YAML_CPP_BINARY_DIR})

  if (EKAT_DISABLE_TPL_WARNINGS)
    include (EkatUtils)
    EkatDisableAllWarning(yaml-cpp)
  endif ()
else()
  get_target_property(PREV_YAML_CPP_SOURCE_DIR YAML_CPP SOURCE_DIR)
  message (STATUS "yaml-cpp target already added in ${PREV_YAML_CPP_SOURCE_DIR}")
endif()
