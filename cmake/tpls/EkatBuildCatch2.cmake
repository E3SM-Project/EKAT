# Fetch Catch2 (single_include) via FetchContent
include (FetchContent)
include (EkatUtils)

message (STATUS "  Fetching Catch2 via FetchContent")

set (CATCH2_GIT_TAG 0d70154ddf178cf99bca140669e0618267e51682)

# Do not allow the user to enable these
set(CATCH_BUILD_TESTING OFF CACHE BOOL "Disable Catch2 internal tests" FORCE)
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "Disable Catch2 install" FORCE)
set(CATCH_FORCE_INSTALL ON CACHE BOOL "Force install even if Catch2 is a subproject" FORCE)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "Install documentation alongside library" FORCE)

# If TPL is already present with correct sha simply adds subdir, otherwise uses FetchContent first
ekat_fetch_content(Catch2
  GIT_REPOSITORY https://github.com/E3SM-Project/Catch2.git
  GIT_TAG        ${CATCH2_GIT_TAG}
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/Catch2
)
