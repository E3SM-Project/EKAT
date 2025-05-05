include(GNUInstallDirs)

# Create the library, and set all its properties
add_library(TestUtils ekat_test_utils.cpp)
target_link_libraries(TestUtils PUBLIC Core Utils)

target_include_directories(TestUtils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_test_utils.hpp
)
set_target_properties(TestUtils PROPERTIES PUBLIC_HEADER "${HEADERS}")

# A small lib for Catch2 main executable, so we don't rebuild it
# for every single unit test
add_library(CatchMain ekat_catch_main.cpp)
target_link_libraries(CatchMain PUBLIC TestUtils Session)
target_include_directories(CatchMain PUBLIC
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/extern/Catch2/single_include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# This allows users to have unit tests with reasonable FPE's enabled without having
# to manually call ekat::enable_fpes(..) in their tests
option (EKAT_ENABLE_FPE_DEFAULT_MASK "Whether to enable some reasonable FPE mask for testing" OFF)
if (EKAT_ENABLE_FPE_DEFAULT_MASK)
  target_compile_definitions(CatchMain PUBLIC EKAT_ENABLE_FPE_DEFAULT_MASK)
endif()


# Install the packages
install (TARGETS TestUtils CatchMain
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install catch2 header
install (DIRECTORY ${EKAT_SOURCE_DIR}/extern/Catch2/single_include/catch2
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
