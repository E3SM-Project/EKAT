# Fetch Catch2 (single_include) via FetchContent
include (FetchContent)

message (STATUS "  Fetching Catch2 via FetchContent")

# Do not allow the user to enable these
set(CATCH_BUILD_TESTING OFF CACHE BOOL "Disable Catch2 internal tests" FORCE)
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "Disable Catch2 install" FORCE)
set(CATCH_FORCE_INSTALL ON CACHE BOOL "Force install even if Catch2 is a subproject" FORCE)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "Install documentation alongside library" FORCE)

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/E3SM-Project/Catch2.git
  GIT_TAG        0d70154ddf178cf99bca140669e0618267e51682
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/Catch2
  BINARY_DIR     ${EKAT_BINARY_DIR}/extern/Catch2
)

# MakeAvailable is better, but we need CMake>=3.14...
if(CMAKE_VERSION VERSION_LESS "3.14")
  # Legacy path for CMake 3.11 - 3.13
  FetchContent_GetProperties(Catch2)
  if(NOT catch2_POPULATED)
    FetchContent_Populate(Catch2)
    add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
  endif()
else()
  # Modern path for 3.14+ (Silences CMP0169 in 3.30+)
  FetchContent_MakeAvailable(Catch2)
endif()
