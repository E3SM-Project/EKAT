# Fetch and build spdlog via FetchContent
include (FetchContent)

message (STATUS "  Fetching spdlog via FetchContent")

set (SPDLOG_GIT_TAG bdd1dff3788ebfe520f48f9ad216c60da6dd8f00)

# We don't want testing or any spdlog executable at all
option (SPDLOG_BUILD_TESTS "Enable spdlog tests" OFF)
option (SPDLOG_BUILD_EXAMPLE "Enable spdlog examples" OFF)
option (SPDLOG_INSTALL "Spdlog install location" ON)

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/e3sm-project/spdlog.git
  GIT_TAG        ${SPDLOG_GIT_TAG}
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/spdlog
  BINARY_DIR     ${EKAT_BINARY_DIR}/extern/spdlog
)

# We must protect the MakeAvailable call with a LOCK,
# to avoid race condition if building 2+ builds at once
# We use a function since the lock FUNCTION guard is the safest
# approach (better than FILE), but needs a function scope
function (ekat_spdlog_fetch_content)
  if (FETCHCONTENT_SOURCE_DIR_SPDLOG)
    message(STATUS "  Using provided spdlog source: ${FETCHCONTENT_SOURCE_DIR_SPDLOG}")
  else()
    # Unique lock per user/path to avoid /tmp permission issues
    set(U_ID "cmake")
    if(DEFINED ENV{USER})
      set(U_ID $ENV{USER})
    endif()
    string(REPLACE "/" "_" SANITIZED_PATH "${EKAT_SOURCE_DIR}")
    set(SPDLOG_LOCK_FILE "/tmp/${U_ID}_ekat_spdlog_${SANITIZED_PATH}.lock")
    file (LOCK "${SPDLOG_LOCK_FILE}" GUARD FUNCTION)

    if (EXISTS "${EKAT_SOURCE_DIR}/extern/spdlog/.git/index")
      # Ask Git for the current HEAD commit hash
      execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY "${EKAT_SOURCE_DIR}/extern/spdlog"
        OUTPUT_VARIABLE ON_DISK_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if (ON_DISK_SHA STREQUAL SPDLOG_GIT_TAG)
        message (STATUS "  spdlog source dir already populated with correct version. Skipping download.")
        set(FETCHCONTENT_SOURCE_DIR_SPDLOG "${EKAT_SOURCE_DIR}/extern/spdlog" CACHE PATH "Path to spdlog source" FORCE)
      else()
        message (STATUS "  spdlog version mismatch (Disk: ${ON_DISK_SHA}, Req: ${SPDLOG_GIT_TAG}). Proceeding with update.")
      endif()
    endif()
  endif()

  FetchContent_MakeAvailable(spdlog)
endfunction()

ekat_spdlog_fetch_content()

if (EKAT_DISABLE_TPL_WARNINGS)
  include (EkatUtils)
  EkatDisableAllWarning(spdlog)
endif ()
