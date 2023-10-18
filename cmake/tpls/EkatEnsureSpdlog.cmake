#Try to find the package
message (STATUS "Looking for spdlog ...")
find_package(spdlog QUIET)

# If not found, download it and add subirectory
if (NOT spdlog_FOUND)
  message (STATUS "Looking for spdlog ... NOT FOUND")
  message (STATUS "  -> Downloading and building locally in ${EKAT_BINARY_DIR}/tpls")
  include (FetchContent)

  # Fetch and populate the external project
  set (FETCHCONTENT_BASE_DIR ${EKAT_BINARY_DIR}/tpls)

  FetchContent_Declare (
    spdlog
    GIT_REPOSITORY git@github.com:gabime/spdlog
    GIT_TAG        76fb40d95455f249bd70824ecfcae7a8f0930fa3
    OVERRIDE_FIND_PACKAGE
  )

  # Set options for yamlcpp before adding the subdirectory
  option (SPDLOG_BUILD_TESTS "Enable spdlog tests" OFF)
  option (SPDLOG_BUILD_EXAMPLE "Enable spdlog examples" OFF)
  option (SPDLOG_INSTALL "Spdlog install location" ON)

  message (STATUS " *** Begin of spdlog configuration ***")
  FetchContent_MakeAvailable (spdlog)
  message (STATUS " ***  End of spdlog configuration  ***")

  if (EKAT_DISABLE_TPL_WARNINGS)
    include (EkatUtils)
    EkatDisableAllWarning(spdlog)
  endif ()
else()
  message (STATUS "Looking for spdlog ... FOUND")
  message (STATUS "  spdlog_DIR: ${spdlog_DIR}")
endif()
