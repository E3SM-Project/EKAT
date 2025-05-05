include(GNUInstallDirs)

# Create the library, and set all its properties
add_library(Utils
  ekat_parameter_list.cpp
  ekat_string_utils.cpp
)
target_link_libraries(Utils PUBLIC Core)

target_include_directories(Utils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set (HEADERS
  ekat_factory.hpp
  ekat_meta_utils.hpp
  ekat_parameter_list.hpp
  ekat_scalar_traits.hpp
  ekat_std_any.hpp
  ekat_std_enable_shared_from_this.hpp
  ekat_std_map_key_iterator.hpp
  ekat_std_type_traits.hpp
  ekat_std_utils.hpp
  ekat_string_utils.hpp
  ekat_type_traits.hpp
)

# Set the PUBLIC_HEADER property
set_target_properties(Utils PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS Utils
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
