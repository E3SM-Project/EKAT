include(GNUInstallDirs)

# A small lib for Catch2 main executable, so we don't rebuild it
# for every single unit test
add_library(ekat_catchmain ekat_catch_main.cpp)
target_link_libraries(ekat_catchmain PUBLIC ekat::Core)
target_include_directories(ekat_catchmain PUBLIC
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/extern/Catch2/single_include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if (TARGET KokkosUtils)
  target_link_libraries(ekat_catchmain PUBLIC KokkosUtils)
endif()

# This allows users to have unit tests with reasonable FPE's enabled without having
# to manually call ekat::enable_fpes(..) in their tests
option (EKAT_ENABLE_FPE_DEFAULT_MASK "Whether to enable some reasonable FPE mask for testing" OFF)
if (EKAT_ENABLE_FPE_DEFAULT_MASK)
  target_compile_definitions(ekat_catchmain PUBLIC EKAT_ENABLE_FPE_DEFAULT_MASK)
endif()

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::CatchMain
set_target_properties(ekat_catchmain PROPERTIES
  EXPORT_NAME CatchMain
  PUBLIC_HEADER "${HEADERS}")

# Install the packages
install (TARGETS ekat_catchmain
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install catch2 header
install (DIRECTORY ${EKAT_SOURCE_DIR}/extern/Catch2/single_include/catch2
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::CatchMain ALIAS ekat_catchmain)
