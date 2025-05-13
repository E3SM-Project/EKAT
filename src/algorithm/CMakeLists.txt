include(GNUInstallDirs)

# Create the library, and set all its properties
# NOTE: since there are NO .cpp files, we must declare the target as INTERFACE
add_library(ekat_algorithm INTERFACE)
target_link_libraries(ekat_algorithm INTERFACE ekat::KokkosUtils ekat::Pack)

target_include_directories(ekat_algorithm INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)

# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_lin_interp.hpp
  ekat_lin_interp_impl.hpp
  ekat_tridiag.hpp
  ekat_reduction_utils.hpp
)
set_target_properties(ekat_algorithm PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::Algorithm
set_target_properties(ekat_algorithm PROPERTIES
  EXPORT_NAME Algorithm
  PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS ekat_algorithm
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::Algorithm ALIAS ekat_algorithm)
