include(GNUInstallDirs)

# A small lib for Catch2 main executable, so we don't rebuild it
# for every single unit test
add_library(CatchMain ekat_catch_main.cpp)
target_link_libraries(CatchMain PUBLIC Core)
target_include_directories(CatchMain PUBLIC
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/extern/Catch2/single_include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if (TARGET KokkosUtils)
  target_link_libraries(CatchMain PUBLIC KokkosUtils)
endif()

# This allows users to have unit tests with reasonable FPE's enabled without having
# to manually call ekat::enable_fpes(..) in their tests
option (EKAT_ENABLE_FPE_DEFAULT_MASK "Whether to enable some reasonable FPE mask for testing" OFF)
if (EKAT_ENABLE_FPE_DEFAULT_MASK)
  target_compile_definitions(CatchMain PUBLIC EKAT_ENABLE_FPE_DEFAULT_MASK)
endif()

# Install the packages
install (TARGETS CatchMain
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install catch2 header
install (DIRECTORY ${EKAT_SOURCE_DIR}/extern/Catch2/single_include/catch2
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
