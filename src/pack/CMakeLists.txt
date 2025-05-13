include(GNUInstallDirs)

# Create the library, and set all its properties
# NOTE: since there are NO .cpp files, we must declare the target as INTERFACE
add_library(ekat_pack INTERFACE)
target_link_libraries(ekat_pack INTERFACE ekat::Core ekat::KokkosUtils)

target_include_directories(ekat_pack INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)

set (HEADERS
  ekat_pack_macros.hpp
  ekat_pack.hpp
  ekat_pack_math.hpp
  ekat_pack_utils.hpp
  ekat_pack_kokkos.hpp
  ekat_pack_where.hpp
)

# Set the PUBLIC_HEADER property
set_target_properties(ekat_pack
  PROPERTIES PUBLIC_HEADER "${HEADERS}"
)

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::Pack
set_target_properties(ekat_pack PROPERTIES
  EXPORT_NAME Pack
  PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS ekat_pack
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::Pack ALIAS ekat_pack)
