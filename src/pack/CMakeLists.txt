include(GNUInstallDirs)

# Create the library, and set all its properties
# NOTE: since there are NO .cpp files, we must declare the target as INTERFACE
add_library(Pack INTERFACE)
target_link_libraries(Pack INTERFACE Core KokkosUtils)

target_include_directories(Pack INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set (HEADERS
  ekat_pack_macros.hpp
  ekat_pack.hpp
  ekat_pack_math.hpp
  ekat_pack_utils.hpp
  ekat_pack_kokkos.hpp
  ekat_pack_where.hpp
)

# Set the PUBLIC_HEADER property
set_target_properties(Pack
  PROPERTIES PUBLIC_HEADER "${HEADERS}"
)

# Install the package
install (TARGETS Pack
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

