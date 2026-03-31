# Fetch Catch2 (single_include) via FetchContent
include (FetchContent)

message (STATUS "No Catch2 headers found. Fetching via FetchContent")

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/E3SM-Project/Catch2.git
  GIT_TAG        958944d27a2d2fb82aa008377bf4f8752f6b848e
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/extern/Catch2
)

FetchContent_GetProperties(Catch2)
if (NOT catch2_POPULATED)
  message (STATUS "  Downloading Catch2 to: ${CMAKE_SOURCE_DIR}/extern/Catch2")
  FetchContent_Populate(Catch2)
  # We only need the single_include directory; no need to add_subdirectory
endif ()
