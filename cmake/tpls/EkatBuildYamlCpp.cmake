# Fetch and build yaml-cpp via FetchContent
include (FetchContent)

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

# We must protect the MakeAvailable call with a LOCK,
# to avoid race condition if building 2+ builds at once
# We use a function since the lock FUNCTION guard is the safest
# approach (better than FILE), but needs a function scope
function (ekat_yaml_cpp_fetch_content)
  if (FETCHCONTENT_SOURCE_DIR_YAML_CPP)
    message(STATUS "  Using provided yaml-cpp source: ${FETCHCONTENT_SOURCE_DIR_YAML_CPP}")
  else()
    # Unique lock per user/path to avoid /tmp permission issues
    set(U_ID "cmake")
    if(DEFINED ENV{USER})
      set(U_ID $ENV{USER})
    endif()
    string(REPLACE "/" "_" SANITIZED_PATH "${EKAT_SOURCE_DIR}")
    set(YAML_CPP_LOCK_FILE "/tmp/${U_ID}_ekat_yaml-cpp_${SANITIZED_PATH}.lock")
    file (LOCK "${YAML_CPP_LOCK_FILE}" GUARD FUNCTION)

    if (EXISTS "${EKAT_SOURCE_DIR}/extern/yaml-cpp/.git/index")
      # Ask Git for the current HEAD commit hash
      execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY "${EKAT_SOURCE_DIR}/extern/yaml-cpp"
        OUTPUT_VARIABLE ON_DISK_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if (ON_DISK_SHA STREQUAL YAML_CPP_GIT_TAG)
        message (WARNING "  yaml-cpp source dir already populated with correct version. Skipping download.")
        set(FETCHCONTENT_SOURCE_DIR_YAML_CPP "${EKAT_SOURCE_DIR}/extern/yaml-cpp" CACHE PATH "Path to yaml-cpp source" FORCE)
      else()
        message (STATUS "  yaml-cpp version mismatch (Disk: ${ON_DISK_SHA}, Req: ${YAML_CPP_GIT_TAG}). Proceeding with update.")
      endif()
    endif()
  endif()

  FetchContent_MakeAvailable(yaml-cpp)
endfunction()

ekat_yaml_cpp_fetch_content()

if (EKAT_DISABLE_TPL_WARNINGS)
  include (EkatUtils)
  EkatDisableAllWarning(yaml-cpp)
endif ()

if (NOT TARGET yaml-cpp::yaml-cpp)
  # We want to guarantee that the scoped target is defined
  add_library (yaml-cpp::yaml-cpp ALIAS yaml-cpp)
endif()
