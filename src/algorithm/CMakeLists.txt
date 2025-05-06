include(GNUInstallDirs)

# Create the library, and set all its properties
# NOTE: since there are NO .cpp files, we must declare the target as INTERFACE
add_library(Algorithm INTERFACE)
target_link_libraries(Algorithm INTERFACE KokkosUtils Pack)

target_include_directories(Algorithm INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set (HEADERS
  ekat_lin_interp.hpp
  ekat_lin_interp_impl.hpp
  ekat_tridiag.hpp
  ekat_reduction_utils.hpp
)

# Set the PUBLIC_HEADER property
set_target_properties(Algorithm PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS Algorithm
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
