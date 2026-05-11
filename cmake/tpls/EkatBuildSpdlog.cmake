# Fetch and build spdlog via FetchContent
include (FetchContent)
include (EkatUtils)

message (STATUS "  Fetching spdlog via FetchContent")

# spdlog version 1.17.0 commit sha
set (SPDLOG_GIT_TAG 79524ddd08a4ec981b7fea76afd08ee05f83755d)

# We don't want testing or any spdlog executable at all
option (SPDLOG_BUILD_TESTS "Enable spdlog tests" OFF)
option (SPDLOG_BUILD_EXAMPLE "Enable spdlog examples" OFF)
option (SPDLOG_INSTALL "Spdlog install location" ON)

# If TPL is already present with correct sha simply adds subdir, otherwise uses FetchContent first
ekat_fetch_content(spdlog
  GIT_REPOSITORY https://github.com/e3sm-project/spdlog.git
  GIT_TAG        ${SPDLOG_GIT_TAG}
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/spdlog
)

if (EKAT_DISABLE_TPL_WARNINGS)
  EkatDisableAllWarning(spdlog)
endif ()
