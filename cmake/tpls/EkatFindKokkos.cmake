# Before looking for Kokkos, gather all Kokkos_ CACHE Vars currently set.
# If find_package finds Kokkos, we only accept it if the list of kokkos
# options/devices/archs is indeed enabled in the installation
get_cmake_property(var_names VARIABLES)

set (VALID_DEVICES_OPTIONS
  Kokkos_ENABLE_CUDA
  Kokkos_ENABLE_HIP
  Kokkos_ENABLE_SYCL
  Kokkos_ENABLE_OPENMP
  Kokkos_ENABLE_SERIAL
)
foreach (var_name IN LISTS var_names)
  # If a device, store dev name
  list (FIND VALID_DEVICES_OPTIONS ${var_name} index)
  if (index GREATER -1)
    string (REPLACE "Kokkos_ENABLE_" "" DEVICE ${var_name})
    if (Kokkos_ENABLE_${DEVICE})
      message ("must find dev: ${DEVICE}, because Kokkos_ENABLE_${DEVICE}=${Kokkos_ENABLE_${DEVICE}}")
      list (APPEND EKAT_FIND_KOKKOS_DEVICES ${DEVICE})
    endif()
    continue()
  endif()

  # If an arch, store arch name
  string(REGEX MATCH "Kokkos_ARCH_" MATCH ${var_name})
  if ("${MATCH}" AND ${var_name})
    string (REPLACE "Kokkos_ARCH_" "" ARCH ${var_name})
    if (Kokkos_ARCH_${ARCH})
      list (APPEND EKAT_FIND_KOKKOS_ARCHS ${ARCH})
    endif()
    continue()
  endif()

  # If any other Kokkos_ENABLE_XYZ, store as option
  string(REGEX MATCH "Kokkos_ENABLE_" MATCH ${var_name})
  if ("${MATCH}" AND ${var_name})
    string (REPLACE "Kokkos_ENABLE_" "" OPTION ${var_name})
    if (Kokkos_ENABLE_${OPTION})
      list (APPEND EKAT_FIND_KOKKOS_OPTIONS ${OPTION})
    endif()
  endif()
endforeach()

# Try to find the package
message (STATUS "Looking for a Kokkos installation ...")
find_package(Kokkos QUIET)

if (Kokkos_FOUND)
  # Check if the requested devices/archs/options are enabled in the installation
  foreach (dev IN ITEMS ${EKAT_FIND_KOKKOS_DEVICES})
    list (FIND Kokkos_DEVICES ${dev} index)
    if (index EQUAL -1)
      list (APPEND MISSING_DEVICES ${dev})
      set (Kokkos_FOUND FALSE)
    endif()
  endforeach()
  foreach (arch IN ITEMS ${EKAT_FIND_KOKKOS_ARCHS})
    list (FIND Kokkos_ARCH ${arch} index)
    if (index EQUAL -1)
      list (APPEND MISSING_ARCHS ${arch})
      set (Kokkos_FOUND FALSE)
    endif()
  endforeach()
  foreach (opt IN ITEMS ${EKAT_FIND_KOKKOS_OPTIONS})
    list (FIND Kokkos_OPTIONS ${opt} index)
    if (index EQUAL -1)
      list (APPEND MISSING_OPTIONS ${arch})
      set (Kokkos_FOUND FALSE)
    endif()
  endforeach()
endif()

# If not found, download it and add subirectory
if (NOT Kokkos_FOUND)
  message (STATUS "Looking for a Kokkos installation ... NOT FOUND")
  if (MISSING_DEVICES OR MISSING_ARCHS OR MISSING_OPTIONS)
    message (STATUS "  -> An installation was found at ${Kokkos_DIR}, but was not accepted, because:")

    if (MISSING_DEVICES)
      message (STATUS "   - Requested devices ${MISSING_DEVICES} not found in the installation")
    endif()
    if (MISSING_ARCHS)
      message (STATUS "   - Requested archs ${MISSING_ARCHS} not found in the installation")
    endif()
    if (MISSING_OPTIONS)
      message (STATUS "   - Requested options ${MISSING_OPTIONS} not found in the installation")
    endif()
  endif()
else()
  message (STATUS "Looking for a Kokkos installation ... FOUND")
  message (STATUS "  Kokkos_DIR: ${Kokkos_DIR}")
endif()
