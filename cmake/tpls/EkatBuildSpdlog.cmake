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

FetchContent_MakeAvailable(spdlog)

if (EKAT_DISABLE_TPL_WARNINGS)
  include (EkatUtils)
  EkatDisableAllWarning(spdlog)
endif ()
