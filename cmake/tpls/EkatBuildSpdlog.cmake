# Fetch and build spdlog via FetchContent
include (FetchContent)

message (STATUS "  Fetching spdlog via FetchContent")

# We don't want testing or any spdlog executable at all
option (SPDLOG_BUILD_TESTS "Enable spdlog tests" OFF)
option (SPDLOG_BUILD_EXAMPLE "Enable spdlog examples" OFF)
option (SPDLOG_INSTALL "Spdlog install location" ON)

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/e3sm-project/spdlog.git
  GIT_TAG        bdd1dff3788ebfe520f48f9ad216c60da6dd8f00
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/spdlog
  BINARY_DIR     ${EKAT_BINARY_DIR}/extern/spdlog
)

# MakeAvailable is better, but we need CMake>=3.14...
if(CMAKE_VERSION VERSION_LESS "3.14")
  # Legacy path for CMake 3.11 - 3.13
  FetchContent_GetProperties(spdlog)
  if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory (${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
  endif()
else()
  # Modern path for 3.14+ (Silences CMP0169 in 3.30+)
  FetchContent_MakeAvailable(spdlog)
endif()

if (EKAT_DISABLE_TPL_WARNINGS)
  include (EkatUtils)
  EkatDisableAllWarning(spdlog)
endif ()
