# Where ekat's tpls live
set (EKAT_KOKKOS_SUBMODULE_PATH "" CACHE INTERNAL "")
get_filename_component(EKAT_KOKKOS_SUBMODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../extern/kokkos ABSOLUTE)

# Define a global property to check if Kokkos has already been built
define_property(GLOBAL
                PROPERTY EKAT_KOKKOS_BUILT
                BRIEF_DOCS "Whether kokkos subdir has already been processed"
                FULL_DOCS "This property is used by cmake to ensure that Kokkos
                           submodule directory is only processed once (with add_subdirectory).")

get_property(IS_EKAT_KOKKOS_BUILT GLOBAL PROPERTY EKAT_KOKKOS_BUILT SET)

# Make sure Kokkos_SOURCE_DIR is set. If not set, default to using submodule
macro(EkatSetKokkosSourceDir)
  if (NOT Kokkos_SOURCE_DIR)
    message (STATUS "Kokkos_SOURCE_DIR not specified: using submodule version.")
    set (Kokkos_SOURCE_DIR "${EKAT_KOKKOS_SUBMODULE_PATH}" CACHE STRING "Kokkos source directory")
    if(Kokkos_ENABLE_DEPRECATED_CODE)
      message(FATAL_ERROR "Kokkos submodule cannot be used with\n"
                          "Kokkos_ENABLE_DEPRECATED_CODE.")
    endif()
  elseif (NOT EXISTS ${Kokkos_SOURCE_DIR})
    message (FATAL_ERROR "Error! Please specify a valid source folder for kokkos.\n"
                         "       Provided path: ${Kokkos_SOURCE_DIR}")
  else ()
    get_filename_component(ABS_KOKKOS_DIR ${Kokkos_SOURCE_DIR} ABSOLUTE)
    if(ABS_KOKKOS_DIR STREQUAL EKAT_KOKKOS_SUBMODULE_PATH)
      message (STATUS "Using Kokkos in ${Kokkos_SOURCE_DIR}\n"
                   "    - User-supplied Kokkos path matches submodule path.")
    else ()
      message (STATUS "Using Kokkos in ${Kokkos_SOURCE_DIR}\n"
                      "User-supplied Kokkos versions are not guaranteed to work.")
    endif()
  endif()

  # IF the variable existed, but not in the cache, set it in the cache
  set (Kokkos_SOURCE_DIR "${Kokkos_SOURCE_DIR}" CACHE STRING "Kokkos source directory")
endmacro()

# Process the kokkos source directory
macro(BuildKokkos)
  if (NOT IS_EKAT_KOKKOS_BUILT)

    # Make sure Kokkos_SOURCE_DIR is set
    EkatSetKokkosSourceDir()

    set(Kokkos_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/kokkos)

    # Enable Kokkos debug if the host project is in debug mode.
    if (CMAKE_BUILD_TYPE_ci STREQUAL "debug")
      set(Kokkos_ENABLE_DEBUG              TRUE CACHE BOOL "Enable Kokkos Debug")
      set(Kokkos_ENABLE_DEBUG_BOUNDS_CHECK TRUE CACHE BOOL "Enable Kokkos Debug bounds checking")
    else()
      set(Kokkos_ENABLE_DEBUG              FALSE CACHE BOOL "Enable Kokkos Debug")
      set(Kokkos_ENABLE_DEBUG_BOUNDS_CHECK FALSE CACHE BOOL "Enable Kokkos Debug bounds checking")
    endif()

    add_subdirectory(${Kokkos_SOURCE_DIR} ${Kokkos_BINARY_DIR})

    set (Kokkos_INCLUDE_DIRS
      ${Kokkos_SOURCE_DIR}/core/src
      ${Kokkos_SOURCE_DIR}/algorithms/src
      ${Kokkos_BINARY_DIR}
    )
    set(Kokkos_LIBRARIES kokkos)

    if (EKAT_DISABLE_TPL_WARNINGS)
      include (EkatUtils)
      EkatDisableAllWarning(kokkoscore)
      EkatDisableAllWarning(kokkoscontainers)
    endif ()

    # Make sure it is processed only once
    set_property(GLOBAL PROPERTY EKAT_KOKKOS_BUILT TRUE)
  endif ()
endmacro()
