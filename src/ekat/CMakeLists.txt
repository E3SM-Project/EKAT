set(EKAT_SOURCES
  mpi/ekat_comm.cpp
  ekat_assert.cpp
  ekat_session.cpp
  ekat_parameter_list.cpp
  ekat_parse_yaml_file.cpp
  util/ekat_array_io.cpp
  util/ekat_array_io_mod.f90
  util/ekat_string_utils.cpp
  util/ekat_arch.cpp
  util/ekat_test_utils.cpp
)

add_library(ekat ${EKAT_SOURCES})
install(TARGETS ekat DESTINATION lib)

# Make sure headers can be installed.
set(EKAT_HEADERS
  ekat_pack.hpp
  ekat_parse_yaml_file.hpp
  ekat_workspace.hpp
  ekat_pack_kokkos.hpp
  ekat_scalar_traits.hpp
  ekat_workspace_impl.hpp
  ekat_assert.hpp
  ekat.hpp
  ekat_parameter_list.hpp
  ekat_session.hpp
  ekat_macros.hpp
  ekat_type_traits.hpp
)
install(FILES ${EKAT_HEADERS} DESTINATION include/ekat)
set(EKAT_KOKKOS_HEADERS
  kokkos/ekat_kokkos_meta.hpp
  kokkos/ekat_kokkos_types.hpp
  kokkos/ekat_kokkos_utils.hpp
)
install(FILES ${EKAT_KOKKOS_HEADERS} DESTINATION include/ekat/kokkos)
install(FILES mpi/ekat_comm.hpp DESTINATION include/ekat/mpi)
set(EKAT_STD_META_HEADERS
  std_meta/ekat_std_any.hpp
  std_meta/ekat_std_enable_shared_from_this.hpp
  std_meta/ekat_std_type_traits.hpp
  std_meta/ekat_std_utils.hpp
)
install(FILES ${EKAT_STD_META_HEADERS} DESTINATION include/ekat/std_meta)
set(EKAT_STD_UTIL_HEADERS
  util/ekat_feutils.hpp
  util/ekat_rational_constant.hpp
  util/ekat_test_utils.hpp
  util/ekat_arch.hpp
  util/ekat_file_utils.hpp
  util/ekat_scaling_factor.hpp
  util/ekat_tridiag.hpp
  util/ekat_lin_interp.hpp
  util/ekat_units.hpp
  util/ekat_lin_interp_impl.hpp
  util/ekat_string_utils.hpp
  util/ekat_upper_bound.hpp
  util/ekat_math_utils.hpp
  util/ekat_factory.hpp
  util/ekat_md_array.hpp
)
install(FILES ${EKAT_UTIL_HEADERS} DESTINATION include/ekat/util)

target_include_directories(ekat PUBLIC ${EKAT_INCLUDE_DIRS})
target_include_directories(ekat SYSTEM PUBLIC ${EKAT_TPL_INCLUDE_DIRS})

# TODO: The current version of kokkos we are using in e3sm has a nvcc_wrapper
#       which breaks the cmake toolchain features detection mechanisms.
#       If we update kokkos to, say, current develop (as of 06/2020),
#       the following line would work fine.
# target_compile_features(ekat PUBLIC cxx_std_11)

# Enable C Pre-Processor for Fortran.
# No reason to pollute C/C++ flags (CPP always runs), so add the flag only for Fortran files.
target_compile_options(ekat PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-cpp>)

target_link_libraries(ekat PUBLIC ${EKAT_TPL_LIBRARIES})
set_target_properties(ekat PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ekat_f90_modules)

###########################################
###       Testing micro-libraries       ###
###########################################

# Pre-compile ekat_catch_main.cpp into its own mini-library, so that individual
# unit test don't have to all pointlessly rebuild the same file.
add_library(ekat_test_main util/ekat_catch_main.cpp)
target_link_libraries(ekat_test_main PUBLIC ekat)
target_include_directories(ekat_test_main PUBLIC ${EKAT_SOURCE_DIR}/extern/catch2/include)
install(TARGETS ekat_test_main DESTINATION lib)

# Pre-compile ekat_test_session.cpp into its own mini-library.
# This mini-library has to be linked if you are ok relying on Ekat's test session
# initialization/finalization routines (which simply call initialize_ekat_session
# and finalize_ekat_session respectively). If your test needs to do more stuff
# at init/finalize time, then you should NOT link this mini library, and provide
# init/finalize routines in a separate source file (to be linked in).
# unit test don't have to all pointlessly rebuild the same file.
add_library(ekat_test_session util/ekat_test_session.cpp)
target_link_libraries(ekat_test_session PUBLIC ekat)
target_include_directories(ekat_test_session PUBLIC ${EKAT_SOURCE_DIR}/extern/catch2/include)
install(TARGETS ekat_test_session DESTINATION lib)
