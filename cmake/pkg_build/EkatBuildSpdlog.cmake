# Where ekat's tpls live
set (EKAT_SPDLOG_SUBMODULE_PATH "" CACHE INTERNAL "")
get_filename_component(EKAT_SPDLOG_SUBMODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../extern/spdlog ABSOLUTE)

# Define a global property to check if yamlcpp has already been built
define_property(GLOBAL
                PROPERTY EKAT_SPDLOG_BUILT
                BRIEF_DOCS "Whether spdlog subdir has already been processed"
                FULL_DOCS "This property is used by cmake to ensure that spdlog
                           submodule directory is only processed once (with add_subdirectory).")

get_property(IS_EKAT_SPDLOG_BUILT GLOBAL PROPERTY EKAT_SPDLOG_BUILT SET)

# Make sure SPDLOG_SOURCE_DIR is set. If not set, default to using submodule
macro (EkatSetSpdlogSourceDir)
  if (NOT SPDLOG_SOURCE_DIR)
    message (STATUS "SPDLOG_SOURCE_DIR not specified: using submodule version.")
    set (SPDLOG_SOURCE_DIR "${EKAT_SPDLOG_SUBMODULE_PATH}" CACHE STRING "spdlog source directory")
  elseif (NOT EXISTS ${SPDLOG_SOURCE_DIR})
    message (FATAL_ERROR "Error! Please specify a valid source folder for yamlcpp.\n"
                         "       Provided path: ${SPDLOG_SOURCE_DIR}")
  else()
    get_filename_component(ABS_SPDLOG_DIR ${SPDLOG_SOURCE_DIR} ABSOLUTE)
    if(ABS_SPDLOG_DIR STREQUAL EKAT_SPDLOG_SUBMODULE_PATH)
      message (STATUS "Using spdlog in ${SPDLOG_SOURCE_DIR}.\n"
                   "    - User-supplied spdlog path matches submodule path.")
    else ()
      message (STATUS "Using spdlog in ${SPDLOG_SOURCE_DIR}.\n"
                      "User-supplied spdlog versions are not guaranteed to work.")
    endif()
  endif()

  # If the variable existed, but not in the cache, set it in the cache
  set (SPDLOG_SOURCE_DIR "${SPDLOG_SOURCE_DIR}" CACHE STRING "spdlog source directory")
endmacro()

# Process the spdlog subdirectory
macro (BuildSpdlog)
  if (NOT IS_SPDLOG_ALREADY_BUILT)

    # Make sure SPDLOG_SOURCE_DIR is set
    EkatSetSpdlogSourceDir()

    set (SPDLOG_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/spdlog)

    # We don't want testing or any spdlog executable at all
    option (SPDLOG_BUILD_TESTS "Enable spdlog tests" OFF)
    option (SPDLOG_BUILD_EXAMPLE "Enable spdlog examples" OFF)
    option (SPDLOG_INSTALL "Spdlog install location" ON)
    add_subdirectory (${SPDLOG_SOURCE_DIR} ${SPDLOG_BINARY_DIR})

    set (SPDLOG_INCLUDE_DIRS
       $<BUILD_INTERFACE:${SPDLOG_SOURCE_DIR}/include>
       $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/spdlog/include>)
    set (SPDLOG_LIBRARIES spdlog)

    if (EKAT_DISABLE_TPL_WARNINGS)
      include (EkatUtils)
      EkatDisableAllWarning(spdlog)
    endif ()

    # Make sure it is processed only once
    set_property(GLOBAL PROPERTY EKAT_SPDLOG_BUILT TRUE)
  endif()
endmacro()
