### Kokkos Tools for kernel profiling ###

if (EKAT_ENABLE_KOKKOS_TOOLS)
  set (KokkosTools_FOUND FALSE)
  if (TARGET kokkostools)
    set(KokkosTools_FOUND TRUE)
    message(STATUS "EKAT: Using kokkos-tools provided by parent project")
  else() # not found -- we fetch it ourselves
    include (EkatBuildKokkosTools)
  endif()

  # Build a list of kokkos-tools libraries
  set(profiling_libs simple-kernel-timer)
  foreach (lib_path ${profiling_libs})
    list(APPEND KOKKOS_TOOLS_LIBS ${kokkostools_BINARY_DIR}/profiling/${lib_path})
  endforeach()
endif()

### CATCH MAIN ###

set (Catch2_FOUND FALSE)
if (TARGET Catch2::Catch2 OR TARGET Catch2)
  set(Catch2_FOUND TRUE)
  message(STATUS "EKAT: Using Catch2 provided by parent project")
endif()

# Catch2 is a REQUIRED dep of this package
option (EKAT_SKIP_FIND_CATCH2 "Skip find_package for Catch2, and fetch via FetchContent" OFF)
if (NOT Catch2_FOUND AND NOT EKAT_SKIP_FIND_CATCH2)
  message (STATUS "Looking for Catch2 ...")
  find_package(Catch2 QUIET NO_DEFAULT_PATH)
  if (Catch2_FOUND)
    message (STATUS "Looking for Catch2 ... FOUND")
    message (STATUS "  Catch2_DIR: ${Catch2_DIR}")
  else()
    message (STATUS "Looking for Catch2 ... NOT FOUND")
  endif()
endif()

# If we found Catch2 (via find_package or as an existing tgt)
# ensure it provides the scoped target
if (Catch2_FOUND)
  if (TARGET Catch2 AND NOT TARGET Catch2::Catch2)
    add_library(Catch2::Catch2 ALIAS Catch2)
  endif()
endif()

if (NOT Catch2_FOUND)
  set (EKAT_BUILDS_CATCH2 ON CACHE BOOL "Whether Ekat builds Catch2" FORCE)
  include (EkatBuildCatch2)
else()
  set (EKAT_BUILDS_CATCH2 OFF CACHE BOOL "Whether Ekat builds Catch2" FORCE)
endif()

# A small lib for Catch2 main executable, so we don't rebuild it
# for every single unit test
add_library(ekat_catchmain ekat_catch_main.cpp)
target_link_libraries(ekat_catchmain PUBLIC ekat::Core)
target_link_libraries(ekat_catchmain PUBLIC Catch2::Catch2)

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::CatchMain
set_target_properties(ekat_catchmain PROPERTIES
  EXPORT_NAME CatchMain
)

# Install the packages
install (TARGETS ekat_catchmain
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::CatchMain ALIAS ekat_catchmain)

### TEST SESSION ###

# A small lib implementing default versions of ekat_initialize_test_session() and
# ekat_finalize_test_session functions that ekat_catch_main.cpp requires

add_library(ekat_testsession ekat_test_session.cpp)
target_link_libraries(ekat_testsession PUBLIC ekat::Core)

if (TARGET ekat::KokkosUtils)
  target_link_libraries(ekat_testsession PUBLIC ekat::KokkosUtils)
endif()

# This allows users to have unit tests with reasonable FPE's enabled without having
# to manually call ekat::enable_fpes(..) in their tests
option (EKAT_ENABLE_FPE_DEFAULT_MASK "Whether to enable some reasonable FPE mask for testing" OFF)
if (EKAT_ENABLE_FPE_DEFAULT_MASK)
  target_compile_definitions(ekat_testsession PUBLIC EKAT_ENABLE_FPE_DEFAULT_MASK)
endif()

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::CatchMain
set_target_properties(ekat_testsession PROPERTIES
  EXPORT_NAME DefaultTestSession
)

# If we're using kokkos-tools, define some symbols for ekat_catchmain and ekat_testsession
if (EKAT_ENABLE_KOKKOS_TOOLS)
  target_compile_definitions(ekat_catchmain PRIVATE
    EKAT_ENABLE_KOKKOS_TOOLS
    KOKKOS_TOOLS_LIBS="${KOKKOS_TOOLS_LIBS}"
  )
  target_compile_definitions(ekat_testsession PRIVATE
    EKAT_ENABLE_KOKKOS_TOOLS
    KOKKOS_TOOLS_LIBS="${KOKKOS_TOOLS_LIBS}"
  )
endif()

# Install the packages
install (TARGETS ekat_testsession
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::DefaultTestSession ALIAS ekat_testsession)
