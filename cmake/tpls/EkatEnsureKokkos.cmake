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
    list (APPEND EKAT_FIND_KOKKOS_DEVICES ${DEVICE})
    continue()
  endif()

  # If an arch, store arch name
  string(REGEX MATCH "Kokkos_ARCH_" MATCH ${var_name})
  if ("${MATCH}" AND ${var_name})
    string (REPLACE "Kokkos_ARCH_" "" ARCH ${var_name})
    list (APPEND EKAT_FIND_KOKKOS_ARCHS ${ARCH})
    continue()
  endif()

  # If any other Kokkos_ENABLE_XYZ, store as option
  string(REGEX MATCH "Kokkos_ENABLE_" MATCH ${var_name})
  if ("${MATCH}" AND ${var_name})
    string (REPLACE "Kokkos_ENABLE_" "" OPTION ${var_name})
    list (APPEND EKAT_FIND_KOKKOS_OPTIONS ${OPTION})
  endif()
endforeach()

#Try to find the package
message (STATUS "Looking for Kokkos ...")
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
  message (STATUS "Looking for Kokkos ... NOT FOUND")
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

  message (STATUS "  -> Downloading and building locally in ${EKAT_BINARY_DIR}/tpls")
  include (FetchContent)

  # Fetch and populate the external project
  set (FETCHCONTENT_BASE_DIR ${EKAT_BINARY_DIR}/tpls)

  FetchContent_Declare (
    Kokkos
    GIT_REPOSITORY git@github.com:e3sm-project/kokkos
    GIT_TAG        e8ad274447f11582cef98f0aa409d8ee06d2c7fa
    OVERRIDE_FIND_PACKAGE
  )

  message (STATUS " *** Begin of Kokkos configuration ***")
  FetchContent_MakeAvailable (Kokkos)
  message (STATUS " ***  End of Kokkos configuration  ***")

  include(EkatSetCompilerFlags)
  SetCudaFlags(kokkoscore)
  SetOmpFlags(kokkoscore)
  if (EKAT_DISABLE_TPL_WARNINGS)
    include (EkatUtils)
    EkatDisableAllWarning(kokkoscore)
    EkatDisableAllWarning(kokkoscontainers)
  endif ()
else()
  message (STATUS "Looking for Kokkos ... FOUND")
  message (STATUS "  Kokkos_DIR: ${Kokkos_DIR}")
endif()
