include(GNUInstallDirs)

# Generate configuration file
include (EkatUtils)
EkatConfigFile (${CMAKE_CURRENT_SOURCE_DIR}/ekat_config.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.h
                F90_FILE ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.f)

set(EKAT_SOURCES
  ekat_assert.cpp
  ekat_parameter_list.cpp
  ekat_parse_yaml_file.cpp
  ekat_session.cpp
  mpi/ekat_comm.cpp
  util/ekat_arch.cpp
  util/ekat_array_io.cpp
  util/ekat_array_io_mod.f90
  util/ekat_string_utils.cpp
  util/ekat_test_utils.cpp
)

# Make sure headers can be installed.
set(EKAT_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.h
  ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.f
  ekat.hpp
  ekat_assert.hpp
  ekat_macros.hpp
  ekat_pack.hpp
  ekat_pack_kokkos.hpp
  ekat_pack_utils.hpp
  ekat_parameter_list.hpp
  ekat_parse_yaml_file.hpp
  ekat_scalar_traits.hpp
  ekat_session.hpp
  ekat_type_traits.hpp
  ekat_workspace.hpp
  ekat_workspace_impl.hpp
  kokkos/ekat_kokkos_meta.hpp
  kokkos/ekat_kokkos_types.hpp
  kokkos/ekat_kokkos_utils.hpp
  kokkos/ekat_subview_utils.hpp
  logging/ekat_logger.hpp
  logging/ekat_log_file.hpp
  logging/ekat_log_mpi.hpp
  mpi/ekat_comm.hpp
  std_meta/ekat_std_any.hpp
  std_meta/ekat_std_enable_shared_from_this.hpp
  std_meta/ekat_std_type_traits.hpp
  std_meta/ekat_std_utils.hpp
  util/ekat_arch.hpp
  util/ekat_factory.hpp
  util/ekat_feutils.hpp
  util/ekat_file_utils.hpp
  util/ekat_lin_interp.hpp
  util/ekat_lin_interp_impl.hpp
  util/ekat_math_utils.hpp
  util/ekat_md_array.hpp
  util/ekat_rational_constant.hpp
  util/ekat_scaling_factor.hpp
  util/ekat_string_utils.hpp
  util/ekat_test_utils.hpp
  util/ekat_tridiag.hpp
  util/ekat_units.hpp
  util/ekat_upper_bound.hpp
)

# Create the library, and set all its properties
add_library(ekat ${EKAT_SOURCES})

# To better handle openmp link flags, we need to link the OpenMP cmake targets.
# This is superior to setting openmp compile/link flags, since it works also
# in case of mismatch language between compiler and linker.
# Note: link both CXX and Fortran openmp libs, just in case.
if (EKAT_OPENMP_TARGETS_FOUND)
  target_link_libraries(ekat PUBLIC OpenMP::OpenMP_CXX;OpenMP::OpenMP_Fortran)
endif()

target_include_directories(ekat PUBLIC
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${EKAT_BINARY_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/ekat_f90_modules>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat_f90_modules>)

# TODO: The current version of kokkos we are using in e3sm has a nvcc_wrapper
#       which breaks the cmake toolchain features detection mechanisms.
#       If we update kokkos to, say, current develop (as of 06/2020),
#       the following line would work fine.
# target_compile_features(ekat PUBLIC cxx_std_11)

# Enable C Pre-Processor for Fortran.
# No reason to pollute C/C++ flags (CPP always runs), so add the flag only for Fortran files.
target_compile_options(ekat PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-cpp>)

target_link_libraries(ekat PUBLIC ${EKAT_TPL_LIBRARIES})
set_target_properties(ekat PROPERTIES
    Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ekat_f90_modules)

install (TARGETS ekat
         EXPORT EkatTargets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Unfortunately, adding PUBLIC_HEADER to the install above would flatten the directory
# structure, which would cause include errors (in ekat we keep the directory
# tree in the inlcude, like "ekat/mpi/ekat_comm.hpp")
# Therefore, manually install the src tree (filter only header files), as well
# as the config files in the bin dir
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         FILES_MATCHING PATTERN *.hpp)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.h
               ${CMAKE_CURRENT_BINARY_DIR}/ekat_config.f
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ekat_f90_modules
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

###########################################
###       Testing micro-libraries       ###
###########################################

# Pre-compile ekat_catch_main.cpp into its own mini-library, so that individual
# unit test don't have to all pointlessly rebuild the same file.
add_library(ekat_test_main util/ekat_catch_main.cpp)
target_link_libraries(ekat_test_main PUBLIC ekat)
target_include_directories(ekat_test_main PUBLIC
  $<BUILD_INTERFACE:${EKAT_SOURCE_DIR}/extern/catch2/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


install (TARGETS ekat_test_main
         EXPORT EkatTargets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install (DIRECTORY ${EKAT_SOURCE_DIR}/extern/catch2/include/catch2
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Pre-compile ekat_test_session.cpp into its own mini-library.
# This mini-library has to be linked if you are ok relying on Ekat's test session
# initialization/finalization routines (which simply call initialize_ekat_session
# and finalize_ekat_session respectively). If your test needs to do more stuff
# at init/finalize time, then you should NOT link this mini library, and provide
# init/finalize routines in a separate source file (to be linked in).
# unit test don't have to all pointlessly rebuild the same file.
add_library(ekat_test_session util/ekat_test_session.cpp)
target_link_libraries(ekat_test_session PUBLIC ekat)

install (TARGETS ekat_test_session
         EXPORT EkatTargets
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
