include(GNUInstallDirs)

# MPI is an OPTIONAL dependency of Core (but ON by default)
option (EKAT_ENABLE_MPI "Whether EKAT requires MPI." ON)
if (EKAT_ENABLE_MPI)
  find_package(MPI REQUIRED)

  # NOTE: may be an overkill, but depending on which FindMPI module is called,
  #       if _FIND_REQUIRED is not checked, we may not get a fatal error
  #       if the required components are not found. So check the _FOUND var.
  if (NOT MPI_C_FOUND)
    message (FATAL_ERROR "EKAT *requires* the C component of MPI to be found")
  endif()

  # We should avoid cxx bindings in mpi; they are already deprecated,
  # and can cause headaches at link time, cause they require -lmpi_cxx
  # (openpmi) or -lmpicxx (mpich) flags.
  include(EkatMpiUtils)
  DisableMpiCxxBindings()

  # MPI-related options
  option (EKAT_MPI_ERRORS_ARE_FATAL " Whether EKAT should crash when MPI errors happen." ON)

  set(EKAT_MPIRUN_EXE "mpiexec" CACHE STRING "The executable name for mpirun")
  set(EKAT_MPI_EXTRA_ARGS "--bind-to core" CACHE STRING "Options for mpirun")
  set(EKAT_MPI_NP_FLAG "--map-by" CACHE STRING "The mpirun flag for designating the total number of ranks")
  set(EKAT_MPI_THREAD_FLAG "" CACHE STRING "The mpirun flag for designating the number of threads")
endif()

# Boost stacktrace is an OPTIONAL dependency of Core

# Always looking for stacktrace can cause problems when cross compiling
# Hence, avoid finding boost if user didn't ask for it.
option (EKAT_ENABLE_BOOST_STACKTRACE "Whether to enable Boost stacktrace" OFF)

if (EKAT_ENABLE_BOOST_STACKTRACE)
  message (STATUS "Looking for boost::stacktrace ...")
  # Stacktrace is available with Boost>=1.65
  find_package(Boost 1.65.0 REQUIRED
    COMPONENTS stacktrace_addr2line
  )

  if (Boost_STACKTRACE_ADDR2LINE_FOUND)
    message (STATUS "Looking for boost::stacktrace ... Found")
    message ("    -> EKAT's assert macros will provide a stacktrace.")
    set (EKAT_HAS_STACKTRACE TRUE)
  endif()
else()
  message (STATUS "EKAT_ENABLE_BOOST_STACKTRACE: OFF")
  message ("    -> EKAT's assert macros will NOT provide a stacktrace.")
endif()

# Create the library, and set all its properties
add_library(ekat_core
  ekat_arch.cpp
  ekat_fpe.cpp
  ekat_parameter_list.cpp
  ekat_string_utils.cpp
  ekat_test_utils.cpp
)

# EKAT requires c++20 features
#target_compile_features(ekat_core PUBLIC cxx_std_20)
set (CMAKE_CXX_STANDARD 20 CACHE STRING "")

# Add the correct ekat::Comm impl file, depending on whether MPI is ON/OFF.
if (EKAT_ENABLE_MPI)
  target_sources(ekat_core PRIVATE ekat_comm.cpp)
  target_compile_definitions(ekat_core PUBLIC EKAT_ENABLE_MPI)
  target_link_libraries(ekat_core PUBLIC MPI::MPI_C)
else()
  target_sources(ekat_core PRIVATE ekat_comm_serial.cpp)
endif()

# Set CPP defs and link libs, depending on CMake options
if (EKAT_MPI_ERRORS_ARE_FATAL)
  target_compile_definitions(ekat_core PUBLIC EKAT_MPI_ERRORS_ARE_FATAL)
endif()

if (EKAT_HAS_STACKTRACE)
  target_link_libraries(ekat_core PUBLIC ${Boost_LIBRARIES})
  target_compile_definitions(ekat_core PRIVATE EKAT_HAS_STACKTRACE BOOST_STACKTRACE_USE_ADDR2LINE)
endif()

if (EKAT_DEFAULT_BFB)
  target_compile_definitions(ekat_core PUBLIC EKAT_DEFAULT_BFB)
endif()

target_include_directories(ekat_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ekat>)

include(CheckCXXSymbolExists)
# Most of the FPE stuff was defined in C99, but gnu has some additional non-std
# functions that are helpful (such as feeenableexcept) --- Apple stopped including these
# in 2005, so we have to drop-in replacements for them.
check_cxx_symbol_exists(feenableexcept "fenv.h" EKAT_HAVE_FEENABLEEXCEPT)
if (EKAT_HAVE_FEENABLEEXCEPT)
  # This is needed internally by ekat, so let's keep it private
  target_compile_definitions (ekat_core PRIVATE EKAT_HAVE_FEENABLEEXCEPT)
endif()

# Set the PUBLIC_HEADER property
set (HEADERS
  ekat.hpp
  ekat_arch.hpp
  ekat_assert.hpp
  ekat_comm.hpp
  ekat_factory.hpp
  ekat_fpe.hpp
  ekat_meta_utils.hpp
  ekat_parameter_list.hpp
  ekat_rational_constant.hpp
  ekat_scalar_traits.hpp
  ekat_scaling_factor.hpp
  ekat_std_any.hpp
  ekat_std_enable_shared_from_this.hpp
  ekat_std_map_key_iterator.hpp
  ekat_std_type_traits.hpp
  ekat_std_utils.hpp
  ekat_string_utils.hpp
  ekat_test_utils.hpp
  ekat_type_traits.hpp
  ekat_units.hpp
)
set_target_properties(ekat_core PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Set the name to be used when exportin the target
# This, together with the NAMESPACE property set in the main CMakeLists.txt
# install call, will force user to link ekat::Core
set_target_properties(ekat_core PROPERTIES
  EXPORT_NAME Core
  PUBLIC_HEADER "${HEADERS}")

# Install the package
install (TARGETS ekat_core
         EXPORT EkatTargets
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ekat)

# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::Core ALIAS ekat_core)

# Link to the all libs target
target_link_libraries(ekat_all_libs INTERFACE ekat_core)
