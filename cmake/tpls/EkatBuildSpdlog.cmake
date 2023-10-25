# Build spdlog from the submodule, unless already built in this CMake project
if (NOT TARGET spdlog)
  message (STATUS "No spdlog target already defined. Building locally from submodule")
  set (spdlog_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../extern/spdlog)
  set (spdlog_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/spdlog)

  message (STATUS "  spdlog_SOURCE_DIR: ${spdlog_SOURCE_DIR}")
  message (STATUS "  spdlog_BINARY_DIR: ${spdlog_BINARY_DIR}")

  # We don't want testing or any spdlog executable at all
  option (SPDLOG_BUILD_TESTS "Enable spdlog tests" OFF)
  option (SPDLOG_BUILD_EXAMPLE "Enable spdlog examples" OFF)
  option (SPDLOG_INSTALL "Spdlog install location" ON)

  add_subdirectory (${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})

  if (EKAT_DISABLE_TPL_WARNINGS)
    include (EkatUtils)
    EkatDisableAllWarning(spdlog)
  endif ()
else()
  get_target_property(PREV_spdlog_SOURCE_DIR spdlog SOURCE_DIR)
  message (STATUS "spdlog target already added in ${PREV_spdlog_SOURCE_DIR}")
endif ()
