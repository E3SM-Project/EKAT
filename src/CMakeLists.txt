include(GNUInstallDirs)

option(EKAT_AUTO_ENABLE_DEPENDENCIES  "Automatically enable EKAT packages if needed" OFF)

option(EKAT_ENABLE_ALGORITHM    "Enable EKAT algorithm utilities" OFF)
option(EKAT_ENABLE_KOKKOS       "Enable EKAT kokkos utilities" OFF)
option(EKAT_ENABLE_LOGGING      "Enable EKAT spdlog utilities" OFF)
option(EKAT_ENABLE_PACK         "Enable EKAT packs utilities" OFF)
option(EKAT_ENABLE_YAML_PARSER  "Enable EKAT yaml parsing utilities" OFF)

# Perform some checks.
macro (EkatCompatibleOptions Option Requires)
  foreach (dep IN ITEMS ${Requires})
    if (EKAT_ENABLE_${Option} AND NOT EKAT_ENABLE_${dep})
      if (EKAT_AUTO_ENABLE_DEPENDENCIES)
        message(STATUS "Setting EKAT_ENABLE_${dep}=ON, since EKAT_ENABLE_${Option}=ON and EKAT_AUTO_ENABLE_DEPENDENCIES=ON")
        set (EKAT_ENABLE_${dep} ON CACHE BOOL "" FORCE)
      else()
        string(CONCAT msg
          "EKAT_ENABLE_${dep} was set to OFF, but EKAT_ENABLE_${Option} requires it.\n"
          "Either set EKAT_ENABLE_${dep}=ON, or EKAT_AUTO_ENABLE_DEPENDENCIES=ON.")
        message ("${msg}")
        message (FATAL_ERROR "Aborting.")
      endif()
    endif()
  endforeach()
endmacro()

# Check packages interdeps
EkatCompatibleOptions(ALGORITHM "PACK;KOKKOS")
EkatCompatibleOptions(PACK "KOKKOS")
    
# Core is ALWAYS enabled
add_subdirectory(core)

# Process optional sub-pacakges as requested
if (EKAT_ENABLE_ALGORITHM)
  add_subdirectory(algorithm)
endif()
if (EKAT_ENABLE_KOKKOS)
  add_subdirectory(kokkos)
endif()
if (EKAT_ENABLE_PACK)
  add_subdirectory(pack)
endif()
if (EKAT_ENABLE_YAML_PARSER)
  add_subdirectory(parser)
endif()

add_subdirectory(catch-main)
