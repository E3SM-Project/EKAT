# Where ekat's tpls live
set (EKAT_CATCH2_SUBMODULE_PATH "" CACHE INTERNAL "")
get_filename_component(EKAT_CATCH2_SUBMODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../extern/Catch2 ABSOLUTE)

# Define a global property to check if yamlcpp has already been built
define_property(GLOBAL
                PROPERTY EKAT_CATCH2_BUILT
                BRIEF_DOCS "Whether Catch2 subdir has already been processed"
                FULL_DOCS "This property is used by cmake to ensure that Catch2
                           submodule directory is only processed once (with add_subdirectory).")

get_property(IS_EKAT_CATCH2_BUILT GLOBAL PROPERTY EKAT_CATCH2_BUILT SET)

# Make sure CATCH2_SOURCE_DIR is set. If not set, default to using submodule
macro (EkatSetCatch2SourceDir)
  if (NOT CATCH2_SOURCE_DIR)
    message (STATUS "CATCH2_SOURCE_DIR not specified: using submodule version.")
    set (CATCH2_SOURCE_DIR "${EKAT_CATCH2_SUBMODULE_PATH}" CACHE STRING "Catch2 source directory")
  elseif (NOT EXISTS ${CATCH2_SOURCE_DIR})
    message (FATAL_ERROR "Error! Please specify a valid source folder for Catch2.\n"
                         "       Provided path: ${CATCH2_SOURCE_DIR}")
  else()
    get_filename_component(ABS_CATCH2_DIR ${CATCH2_SOURCE_DIR} ABSOLUTE)
    if(ABS_CATCH2_DIR STREQUAL EKAT_CATCH2_SUBMODULE_PATH)
      message (STATUS "Using Catch2 in ${CATCH2_SOURCE_DIR}.\n"
                      "    - User-supplied Catch2 path matches submodule path.")
    else ()
      message (STATUS "Using Catch2 in ${CATCH2_SOURCE_DIR}.\n"
                      "User-supplied Catch2 versions are not guaranteed to work.")
    endif()
  endif()

  # IF the variable existed, but not in the cache, set it in the cache
  set (CATCH2_SOURCE_DIR "${CATCH2_SOURCE_DIR}" CACHE STRING "Catch2 source directory")
endmacro()

# Process the Catch2 subdirectory
macro (BuildCatch2)
  if (NOT IS_CATCH2_ALREADY_BUILT)

    # Make sure CATCH2_SOURCE_DIR is set
    EkatSetCatch2SourceDir()

    set (CATCH2_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/Catch2)
    add_subdirectory (${CATCH2_SOURCE_DIR} ${CATCH2_BINARY_DIR})
    install(TARGETS Catch2 EXPORT Catch2Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(EXPORT Catch2Targets DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules)
    set (CATCH2_LIBRARIES Catch2)

    if (EKAT_DISABLE_TPL_WARNINGS)
      include (EkatUtils)
      EkatDisableAllWarning(Catch2)
    endif ()

    # Make sure it is processed only once
    set_property(GLOBAL PROPERTY EKAT_CATCH2_BUILT TRUE)
  endif()
endmacro()
