# Fetch and build yaml-cpp via FetchContent
include (FetchContent)
include (EkatUtils)

message (STATUS "  Fetching yaml-cpp via FetchContent")

set (YAML_CPP_GIT_TAG 95088a0a2b6f2dec0b3e6e59020cdcc0d4f3c658)

# Set options for yamlcpp before adding the subdirectory
option (YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
option (YAML_CPP_BUILD_TESTS "Enable yaml-cpp tests" OFF)

FetchContent_Declare(yaml-cpp
  GIT_REPOSITORY https://github.com/e3sm-project/yaml-cpp.git
  GIT_TAG        ${YAML_CPP_GIT_TAG}
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/yaml-cpp
  BINARY_DIR     ${EKAT_BINARY_DIR}/extern/yaml-cpp
)

# Calls FetchContent_MakeAvailable in a way that avoids race conditions
ekat_make_available(yaml-cpp ${YAML_CPP_GIT_TAG})

if (EKAT_DISABLE_TPL_WARNINGS)
  include (EkatUtils)
  EkatDisableAllWarning(yaml-cpp)
endif ()

if (NOT TARGET yaml-cpp::yaml-cpp)
  # We want to guarantee that the scoped target is defined
  add_library (yaml-cpp::yaml-cpp ALIAS yaml-cpp)
endif()
