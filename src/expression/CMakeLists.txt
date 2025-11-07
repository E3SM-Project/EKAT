add_library(ekat_expression INTERFACE)

target_link_libraries (ekat_expression INTERFACE
  ekat::KokkosUtils)

target_include_directories(ekat_expression INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)

# Set the PUBLIC_HEADER property
set (HEADERS
  ekat_expression.hpp
  ekat_expression_assignment_op.hpp
  ekat_expression_binary_op.hpp
  ekat_expression_compare.hpp
  ekat_expression_conditional.hpp
  ekat_expression_eval.hpp
  ekat_expression_math.hpp
  ekat_expression_meta.hpp
  ekat_expression_view.hpp
)
set_target_properties(ekat_expression PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::Expression
set_target_properties(ekat_expression PROPERTIES
  EXPORT_NAME Expression
  PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS ekat_expression
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::Expression ALIAS ekat_expression)

# Link to the all libs target
target_link_libraries(ekat_all_libs INTERFACE ekat_expression)
