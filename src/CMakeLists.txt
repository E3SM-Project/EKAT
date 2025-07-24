include(GNUInstallDirs)

option(EKAT_ENABLE_ALGORITHM    "Enable EKAT algorithm utilities" OFF)
option(EKAT_ENABLE_KOKKOS       "Enable EKAT kokkos utilities" OFF)
option(EKAT_ENABLE_LOGGING      "Enable EKAT spdlog utilities" OFF)
option(EKAT_ENABLE_PACK         "Enable EKAT packs utilities" OFF)
option(EKAT_ENABLE_YAML_PARSER  "Enable EKAT yaml parsing utilities" OFF)

# Automatically enable pkg A if pkg B is ON and it requires pkg A
macro (EkatEnableDeps Option Requires)
  foreach (dep IN ITEMS ${Requires})
    if (EKAT_ENABLE_${Option} AND NOT EKAT_ENABLE_${dep})
      message(STATUS "Setting EKAT_ENABLE_${dep}=ON, since EKAT_ENABLE_${Option}=ON")
      set (EKAT_ENABLE_${dep} ON CACHE BOOL "" FORCE)
    endif()
  endforeach()
endmacro()

# Check packages interdeps
EkatEnableDeps(PACK "KOKKOS")
EkatEnableDeps(ALGORITHM "PACK;KOKKOS")

# A "rule them all" target, so users can just link against this
add_library(ekat_all_libs INTERFACE)
set_target_properties(ekat_all_libs PROPERTIES
  EXPORT_NAME AllLibs)
# Regardless of whether we use INSTALLED ekat, or BUILT (via add_subdirectory),
# we want to be able to access ekat's targets via ekat::TARGET
add_library(ekat::AllLibs ALIAS ekat_all_libs)

# Core is ALWAYS enabled
add_subdirectory(core)

# Process optional sub-pacakges as requested
if (EKAT_ENABLE_ALGORITHM)
  add_subdirectory(algorithm)
endif()
if (EKAT_ENABLE_KOKKOS)
  add_subdirectory(kokkos)
endif()
if (EKAT_ENABLE_LOGGING)
  add_subdirectory(logging)
endif()
if (EKAT_ENABLE_PACK)
  add_subdirectory(pack)
endif()
if (EKAT_ENABLE_YAML_PARSER)
  add_subdirectory(parser)
endif()

add_subdirectory(testing-support)
