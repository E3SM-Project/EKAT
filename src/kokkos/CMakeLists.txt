include(GNUInstallDirs)

# Kokkos is a REQUIRED dep of this package

option (EKAT_SKIP_FIND_KOKKOS "Skip find_package for Kokkos, and build directly from submodule" OFF)
if (EKAT_SKIP_FIND_KOKKOS)
  # The user does not want to accidentally get an existing installation,
  # and instead wants to force a build from the internal submodule
  include (EkatBuildKokkos)
else()
  # We first try to use find_package (unless told to skip this phase).
  # If that doesn't work, build from submodule
  include (EkatFindKokkos)
  if (NOT Kokkos_FOUND)
    include (EkatBuildKokkos)
  endif()
endif()

# Create the library, and set all its properties
add_library(KokkosUtils
  ekat_kokkos_session.cpp
)
target_link_libraries(KokkosUtils PUBLIC Core)
target_link_libraries(KokkosUtils PUBLIC kokkos)
target_compile_definitions(KokkosUtils PUBLIC EKAT_HAS_KOKKOS)

# A shortcut var, to handle GPU-specific (but backend-agnostic) stuff
if (Kokkos_ENABLE_CUDA OR Kokkos_ENABLE_HIP OR Kokkos_ENABLE_SYCL)
  set (EKAT_ENABLE_GPU TRUE)
  target_compile_definitions(KokkosUtils PUBLIC EKAT_ENABLE_GPU)
endif ()

option (EKAT_MIMIC_GPU "Whether team policies on host should be built with large teams" OFF)
if (EKAT_MIMIC_GPU)
  target_compile_definitions(KokkosUtils PUBLIC EKAT_MIMIC_GPU)
endif()

target_include_directories(KokkosUtils PUBLIC
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
  ekat_team_policy_utils.hpp
  ekat_upper_bound.hpp
  ekat_view_utils.hpp
  ekat_workspace.hpp
  ekat_workspace_impl.hpp
)
set_target_properties(KokkosUtils PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS KokkosUtils
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)
