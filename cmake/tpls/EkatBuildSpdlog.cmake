# Fetch and build spdlog via FetchContent
include (FetchContent)

message (STATUS "Fetching spdlog via FetchContent")

# We don't want testing or any spdlog executable at all
option (SPDLOG_BUILD_TESTS "Enable spdlog tests" OFF)
option (SPDLOG_BUILD_EXAMPLE "Enable spdlog examples" OFF)
option (SPDLOG_INSTALL "Spdlog install location" ON)

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/e3sm-project/spdlog.git
  GIT_TAG        bdd1dff3788ebfe520f48f9ad216c60da6dd8f00
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/extern/spdlog
)

FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
  FetchContent_Populate(spdlog)
  message (STATUS "  spdlog_SOURCE_DIR: ${spdlog_SOURCE_DIR}")
  add_subdirectory (${spdlog_SOURCE_DIR} ${CMAKE_BINARY_DIR}/externals/spdlog)
endif ()

if (EKAT_DISABLE_TPL_WARNINGS)
  include (EkatUtils)
  EkatDisableAllWarning(spdlog)
endif ()
