set(EKAT_SOURCES
  mpi/ekat_comm.cpp
  ekat_assert.cpp
  ekat_session.cpp
  ekat_parameter_list.cpp
  ekat_parse_yaml_file.cpp
  util/ekat_array_io.cpp
  util/array_io_mod.f90
  util/ekat_string_utils.cpp
  util/ekat_arch.cpp
  util/ekat_test_utils.cpp
  util/ekat_time_stamp.cpp
)

add_library(ekat ${EKAT_SOURCES})

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
