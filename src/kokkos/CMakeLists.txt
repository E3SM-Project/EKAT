include(GNUInstallDirs)

# Kokkos is a REQUIRED dep of this package

option (EKAT_SKIP_FIND_KOKKOS "Skip find_package for Kokkos, and build directly from submodule" OFF)
if (EKAT_SKIP_FIND_KOKKOS)
  # The user does not want to accidentally get an existing installation,
  # and instead wants to force a build from the internal submodule
  include (EkatBuildKokkos)
  set (EKAT_BUILDS_KOKKOS ON CACHE BOOL "Whether Ekat builds kokkos" FORCE)
else()
  # We first try to use find_package (unless told to skip this phase).
  # If that doesn't work, build from submodule
  include (EkatFindKokkos)
  if (NOT Kokkos_FOUND)
    include (EkatBuildKokkos)
    set (EKAT_BUILDS_KOKKOS ON CACHE BOOL "Whether Ekat builds kokkos" FORCE)
  else()
    set (EKAT_BUILDS_KOKKOS OFF CACHE BOOL "Whether Ekat builds kokkos" FORCE)
  endif()
endif()

# Create the library, and set all its properties
add_library(ekat_kokkosutils
  ekat_kokkos_session.cpp
)
target_link_libraries(ekat_kokkosutils PUBLIC ekat::Core)
target_link_libraries(ekat_kokkosutils PUBLIC Kokkos::kokkos)
target_compile_definitions(ekat_kokkosutils PUBLIC EKAT_HAS_KOKKOS)

# A shortcut var, to handle GPU-specific (but backend-agnostic) stuff
if (Kokkos_ENABLE_CUDA OR Kokkos_ENABLE_HIP OR Kokkos_ENABLE_SYCL)
  set (EKAT_ENABLE_GPU TRUE)
  target_compile_definitions(ekat_kokkosutils PUBLIC EKAT_ENABLE_GPU)
endif ()

option (EKAT_MIMIC_GPU "Whether team policies on host should be built with large teams" OFF)
if (EKAT_MIMIC_GPU)
  target_compile_definitions(ekat_kokkosutils PUBLIC EKAT_MIMIC_GPU)
endif()

target_include_directories(ekat_kokkosutils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)

# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_kernel_assert.hpp
  ekat_kokkos_meta.hpp
  ekat_kokkos_session.hpp
  ekat_kokkos_types.hpp
  ekat_math_utils.hpp
  ekat_subview_utils.hpp
  ekat_kokkos_str_utils.hpp
  ekat_team_policy_utils.hpp
  ekat_upper_bound.hpp
  ekat_view_utils.hpp
  ekat_where.hpp
  ekat_workspace.hpp
  ekat_workspace_impl.hpp
)
set_target_properties(ekat_kokkosutils PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::KokkosUtils
set_target_properties(ekat_kokkosutils PROPERTIES
  EXPORT_NAME KokkosUtils
  PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS ekat_kokkosutils
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::KokkosUtils ALIAS ekat_kokkosutils)

# Link to the all libs target
target_link_libraries(ekat_all_libs INTERFACE ekat_kokkosutils)
