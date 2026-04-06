# Fetch and build yaml-cpp via FetchContent
include (FetchContent)

message (STATUS "  Fetching yaml-cpp via FetchContent")

# Set options for yamlcpp before adding the subdirectory
option (YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
option (YAML_CPP_BUILD_TESTS "Enable yaml-cpp tests" OFF)

FetchContent_Declare(yaml-cpp
  GIT_REPOSITORY https://github.com/e3sm-project/yaml-cpp.git
  GIT_TAG        95088a0a2b6f2dec0b3e6e59020cdcc0d4f3c658
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/yaml-cpp
  BINARY_DIR     ${EKAT_BINARY_DIR}/extern/yaml-cpp
)

# MakeAvailable is better, but we need CMake>=3.14...
if(CMAKE_VERSION VERSION_LESS "3.14")
  # Legacy path for CMake 3.11 - 3.13
  FetchContent_GetProperties(yaml-cpp)
  if(NOT yaml-cpp_POPULATED)
    FetchContent_Populate(yaml-cpp)
    add_subdirectory (${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
  endif()
else()
  # Modern path for 3.14+ (Silences CMP0169 in 3.30+)
  FetchContent_MakeAvailable(yaml-cpp)
endif()

if (EKAT_DISABLE_TPL_WARNINGS)
  include (EkatUtils)
  EkatDisableAllWarning(yaml-cpp)
endif ()

if (NOT TARGET yaml-cpp::yaml-cpp)
  # We want to guarantee that the scoped target is defined
  add_library (yaml-cpp::yaml-cpp ALIAS yaml-cpp)
endif()
