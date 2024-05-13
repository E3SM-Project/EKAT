# NOTE: unfortunately yaml-cpp does not offer a CMake installation,
#       so we need to find libs/headers by hand
if (NOT TARGET yaml-cpp)
  message (STATUS "No yaml-cpp target already defined. Building locally from submodule")

  # Set options for yamlcpp before adding the subdirectory
  option (YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
  option (YAML_CPP_BUILD_TESTS "Enable yaml-cpp tests" OFF)

  set (yaml-cpp_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../extern/yaml-cpp)
  set (yaml-cpp_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/yaml-cpp)
  message (STATUS "  yaml-cpp_SOURCE_DIR: ${yaml-cpp_SOURCE_DIR}")
  message (STATUS "  yaml-cpp_BINARY_DIR: ${yaml-cpp_BINARY_DIR}")

  add_subdirectory (${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})

  if (EKAT_DISABLE_TPL_WARNINGS)
    include (EkatUtils)
    EkatDisableAllWarning(yaml-cpp)
  endif ()
  if (NOT TARGET yaml-cpp::yaml-cpp)
    # We want to guarantee that the scoped target is defined
    add_library (yaml-cpp::yaml-cpp ALIAS yaml-cpp)
  endif()
else()
  get_target_property(PREV_yaml-cpp_SOURCE_DIR yaml-cpp SOURCE_DIR)
  message (STATUS "yaml-cpp target already added in ${PREV_yaml-cpp_SOURCE_DIR}")
endif()
