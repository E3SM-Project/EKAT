# Fetch Catch2 (single_include) via FetchContent
include (FetchContent)

message (STATUS "  Fetching Catch2 via FetchContent")

set (CATCH2_GIT_TAG 0d70154ddf178cf99bca140669e0618267e51682)

# Do not allow the user to enable these
set(CATCH_BUILD_TESTING OFF CACHE BOOL "Disable Catch2 internal tests" FORCE)
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "Disable Catch2 install" FORCE)
set(CATCH_FORCE_INSTALL ON CACHE BOOL "Force install even if Catch2 is a subproject" FORCE)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "Install documentation alongside library" FORCE)

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/E3SM-Project/Catch2.git
  GIT_TAG        ${CATCH2_GIT_TAG}
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/Catch2
  BINARY_DIR     ${EKAT_BINARY_DIR}/extern/Catch2
)

# We must protect the MakeAvailable call with a LOCK,
# to avoid race condition if building 2+ builds at once
# We use a function since the lock FUNCTION guard is the safest
# approach (better than FILE), but needs a function scope
function (ekat_catch2_fetch_content)
  if (FETCHCONTENT_SOURCE_DIR_CATCH2)
    message(STATUS "  Using provided Catch2 source: ${FETCHCONTENT_SOURCE_DIR_CATCH2}")
  else()
    # Unique lock per user/path to avoid /tmp permission issues
    set(U_ID "cmake")
    if(DEFINED ENV{USER})
      set(U_ID $ENV{USER})
    endif()
    string(REPLACE "/" "_" SANITIZED_PATH "${EKAT_SOURCE_DIR}")
    set(CATCH2_LOCK_FILE "/tmp/${U_ID}_ekat_catch2_${SANITIZED_PATH}.lock")
    file (LOCK "${CATCH2_LOCK_FILE}" GUARD FUNCTION)

    if (EXISTS "${EKAT_SOURCE_DIR}/extern/Catch2/.git/index")
      # Ask Git for the current HEAD commit hash
      execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY "${EKAT_SOURCE_DIR}/extern/Catch2"
        OUTPUT_VARIABLE ON_DISK_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if (ON_DISK_SHA STREQUAL CATCH2_GIT_TAG)
        message (STATUS "  Catch2 source dir already populated with correct version. Skipping download.")
        set(FETCHCONTENT_SOURCE_DIR_CATCH2 "${EKAT_SOURCE_DIR}/extern/Catch2" CACHE PATH "Path to Catch2 source" FORCE)
      else()
        message (STATUS "  Catch2 version mismatch (Disk: ${ON_DISK_SHA}, Req: ${CATCH2_GIT_TAG}). Proceeding with update.")
      endif()
    endif()
  endif()

  FetchContent_MakeAvailable(Catch2)
endfunction()

ekat_catch2_fetch_content()
