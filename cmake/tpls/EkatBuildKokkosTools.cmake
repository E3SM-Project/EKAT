# Fetch kokkos-tools via FetchContent
include (FetchContent)
include (EkatUtils)

message (STATUS "  Fetching kokkos-tools via FetchContent")

set(KOKKOS_TOOLS_GIT_TAG e222e7b6cc7faae5d096e3520d95ca832fb9f889)

# Fixed options
set(KokkosTools_ENABLE_MPI      ON  CACHE BOOL "Enable MPI for kokkos-tools"   FORCE)
set(KokkosTools_ENABLE_EXAMPLES OFF CACHE BOOL "Disable kokkos-tools examples" FORCE)
set(KokkosTools_ENABLE_TESTS    OFF CACHE BOOL "Disable kokkos-tools tests"    FORCE)

# If TPL is already present with correct sha simply adds subdir, otherwise uses FetchContent first
ekat_fetch_content(KokkosTools
  GIT_REPOSITORY https://github.com/kokkos/kokkos-tools.git
  GIT_TAG        ${KOKKOS_TOOLS_GIT_TAG}
  SOURCE_DIR     ${EKAT_SOURCE_DIR}/extern/kokkos-tools
)
