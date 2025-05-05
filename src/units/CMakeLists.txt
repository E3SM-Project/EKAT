include(GNUInstallDirs)

# Create the library, and set all its properties
# NOTE: since there are NO .cpp files, we must declare the target as INTERFACE
add_library(Units INTERFACE)
target_link_libraries(Units PUBLIC Core Utils)

target_include_directories(Units PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set (HEADERS
  ekat_rational_constant.hpp
  ekat_scaling_factor.hpp
  ekat_units.hpp
)

# Set the PUBLIC_HEADER property
set_target_properties(Units PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS Units
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
