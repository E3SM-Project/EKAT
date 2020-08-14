# Define a global property to check if Kokkos has already been built
define_property(GLOBAL
                PROPERTY EKAT_KOKKOS_BUILT
                BRIEF_DOCS "Wheter kokkos subdir has already been processed"
                FULL_DOCS "This property is used by cmake to ensure that Kokkos
                           submodule directory is only processed once (with add_subdirectory).")

get_property(IS_EKAT_KOKKOS_BUILT GLOBAL PROPERTY EKAT_KOKKOS_BUILT SET)

# Process the kokkos source directory
if (NOT IS_EKAT_KOKKOS_BUILT)

  if (NOT Kokkos_SOURCE_DIR)
    message (FATAL_ERROR "Error! Please, specify path to Kokkos in Kokkos_SOURCE_DIR.\n")
  elseif (NOT EXISTS ${Kokkos_SOURCE_DIR})
    message (FATAL_ERROR "Error! Please specify a valid source folder for kokkos.\n"
                         "       Provided path: ${Kokkos_SOURCE_DIR}")
  endif()
  set(Kokkos_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/kokkos)

  # Enable Kokkos debug if the host project is in debug mode.
  if (CMAKE_BUILD_TYPE_ci STREQUAL "debug")
    set(Kokkos_ENABLE_Debug TRUE  CACHE BOOL "Enable Kokkos Debug")
  else()
    set(Kokkos_ENABLE_Debug FALSE CACHE BOOL "Enable Kokkos Debug")
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
    EkatDisableAllWarning(kokkos)
  endif ()

  # Make sure it is processed only once
  set_property(GLOBAL PROPERTY EKAT_KOKKOS_BUILT TRUE)
endif ()
