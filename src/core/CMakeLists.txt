include(GNUInstallDirs)

# Create the library, and set all its properties
add_library(Core ekat_fpe.cpp)

if (EKAT_HAS_STACKTRACE)
  target_link_libraries(Core PUBLIC ${Boost_LIBRARIES})
  target_compile_definitions(Core PRIVATE EKAT_HAS_STACKTRACE)
endif()

target_include_directories(Core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

include(CheckCXXSymbolExists)
# Most of the FPE stuff was defined in C99, but gnu has some additional non-std
# functions that are helpful (such as feeenableexcept) --- Apple stopped including these
# in 2005, so we have to drop-in replacements for them.
check_cxx_symbol_exists(feenableexcept "fenv.h" EKAT_HAVE_FEENABLEEXCEPT)
if (EKAT_HAVE_FEENABLEEXCEPT)
  # This is needed internally by ekat, so let's keep it private
  target_compile_definitions (Core PRIVATE EKAT_HAVE_FEENABLEEXCEPT)
endif()

set (HEADERS
  ekat.hpp
  ekat_assert.hpp
  ekat_fpe.hpp
)

# Set the PUBLIC_HEADER property
set_target_properties(Core PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS Core
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
