#################################################################
#   Process all Ekat's TPLs, ensuring they are found or built   #
#################################################################

message (STATUS " *** Begin processing EKAT's TPLs ***")

option (EKAT_DISABLE_TPL_WARNINGS "Whether we should suppress warnings when compiling TPLs." OFF)

# WARNING: you CANNOT do list(APPEND var item1 ... item2) if var is a CACHE variable!
# Therefore, use an internal var during tpl parsing, then set a cache var ONCE at the end
set (EKAT_TPL_LIBRARIES_INTERNAL)

###################################
#         MPI (Optional)          #
###################################

# MPI is enabled by default, but not needed.
option (EKAT_ENABLE_MPI "Whether EKAT requires MPI." ON)
if (EKAT_ENABLE_MPI)
  find_package(MPI REQUIRED COMPONENTS C)

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

  list (APPEND EKAT_TPL_LIBRARIES_INTERNAL MPI::MPI_C)
endif()

###################################
#        Kokkos (Required)        #
###################################

# We first try to use find_package. If that doesn't work, build from submodule
include (EkatFindKokkos)
if (NOT Kokkos_FOUND)
  include (EkatBuildKokkos)
endif()
list (APPEND EKAT_TPL_LIBRARIES_INTERNAL Kokkos::kokkos)

# A shortcut var, to handle GPU-specific (but backend-agnostic) stuff
set (EKAT_ENABLE_GPU False)
if (Kokkos_ENABLE_CUDA OR Kokkos_ENABLE_HIP OR Kokkos_ENABLE_SYCL)
  set (EKAT_ENABLE_GPU True)
endif ()

###################################
#       yaml-cpp (Optional)       #
###################################

# EKAT also has some yaml parsing utility
option (EKAT_ENABLE_YAML_PARSER "Enable support for parsing YAML files" ON)
if (EKAT_ENABLE_YAML_PARSER)
  # I am having issues getting the env var YAML_CPP_ROOT being picked up
  # by cmake. I suspect this has to do with the presence of the hyphen
  # CMake *should* convert '-' to '_' when forming the cmake/env var name,
  # so YAML_CPP_ROOT should be recognized. Alas, it's not on my machine,
  # running CMake 3.26.3, so we must pass the var explicitly via HINTS
  message (STATUS "Looking for yaml-cpp ...")
  if (NOT YAML_CPP_ROOT AND NOT "$ENV{YAML_CPP_ROOT}" STREQUAL "")
    set (YAML_CPP_ROOT $ENV{YAML_CPP_ROOT})
  endif()

  find_package(yaml-cpp HINTS ${YAML_CPP_ROOT})
  if (NOT yaml-cpp_FOUND)
    message (STATUS "Looking for yaml-cpp ... NOT FOUND")
    include(EkatBuildYamlCpp)
  else()
    message (STATUS "Looking for yaml-cpp ... FOUND")
    message (STATUS "  yaml-cpp_DIR: ${yaml-cpp_DIR}")
  endif()
  list (APPEND EKAT_TPL_LIBRARIES_INTERNAL yaml-cpp)
endif()

###################################
#        spdlog (Required)        #
###################################

# We first try to use find_XYZ. If that doesn't work, build from submodule
message (STATUS "Looking for spdlog ...")
find_package(spdlog QUIET)
if (NOT spdlog_FOUND)
  message (STATUS "Looking for spdlog ... NOT FOUND")
  include(EkatBuildSpdlog)
else()
  message (STATUS "Looking for spdlog ... FOUND")
  message (STATUS "  spdlog_DIR: ${spdlog_DIR}")
endif()
list (APPEND EKAT_TPL_LIBRARIES_INTERNAL spdlog::spdlog)

###################################
#   Boost stacktrace (Optional)   #
###################################

# Stacktrace is available with Boost>=1.65
message (STATUS "Looking for boost::stacktrace ...")
find_package(Boost 1.65.0
  COMPONENTS stacktrace_addr2line
)

if (Boost_STACKTRACE_ADDR2LINE_FOUND)
  list (APPEND EKAT_TPL_LIBRARIES_INTERNAL ${Boost_LIBRARIES})
  message (STATUS "Looking for boost::stacktrace ... Found")
  message ("    -> EKAT's assert macros will provide a stacktrace.")
  set (EKAT_HAS_STACKTRACE TRUE)
else()
  message (STATUS "Looking for boost::stacktrace ... NOT Found")
  message ("    -> EKAT's assert macros will NOT provide a stacktrace.")
endif()

set (EKAT_TPL_LIBRARIES ${EKAT_TPL_LIBRARIES_INTERNAL} CACHE INTERNAL "List of EKAT's TPLs")

message (STATUS " *** End processing EKAT's TPLs ***")
