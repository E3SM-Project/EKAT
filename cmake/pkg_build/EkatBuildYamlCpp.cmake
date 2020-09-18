# Where ekat's tpls live
set (EKAT_YAMLCPP_SUBMODULE_PATH "" CACHE INTERNAL "")
get_filename_component(EKAT_YAMLCPP_SUBMODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../extern/yaml-cpp ABSOLUTE)

# Define a global property to check if yamlcpp has already been built
define_property(GLOBAL
                PROPERTY EKAT_YAMLCPP_BUILT
                BRIEF_DOCS "Whether yaml-cpp subdir has already been processed"
                FULL_DOCS "This property is used by cmake to ensure that yamlcpp
                           submodule directory is only processed once (with add_subdirectory).")

get_property(IS_EKAT_YAMLCPP_BUILT GLOBAL PROPERTY EKAT_YAMLCPP_BUILT SET)

# Make sure YAMLCPP_SOURCE_DIR is set. If not set, default to using submodule
macro (EkatSetYamlcppSourceDir)
  if (NOT YAMLCPP_SOURCE_DIR)
    message (STATUS "YAMLCPP_SOURCE_DIR not specified: using submodule version.")
    set (YAMLCPP_SOURCE_DIR "${EKAT_YAMLCPP_SUBMODULE_PATH}" CACHE STRING "yaml-cpp source directory")
  elseif (NOT EXISTS ${YAMLCPP_SOURCE_DIR})
    message (FATAL_ERROR "Error! Please specify a valid source folder for yamlcpp.\n"
                         "       Provided path: ${YAMLCPP_SOURCE_DIR}")
  else()
    get_filename_component(ABS_YAMLCPP_DIR ${YAMLCPP_SOURCE_DIR} ABSOLUTE)
    if(ABS_YAMLCPP_DIR STREQUAL EKAT_YAMLCPP_SUBMODULE_PATH)
      message (STATUS "Using yaml-cpp in ${YAMLCPP_SOURCE_DIR}.\n"
                   "    - User-supplied yaml-cpp path matches submodule path.")
    else ()
      message (STATUS "Using yaml-cpp in ${YAMLCPP_SOURCE_DIR}.\n"
                      "User-supplied yaml-cpp versions are not guaranteed to work.")
    endif()
  endif()

  # IF the variable existed, but not in the cache, set it in the cache
  set (YAMLCPP_SOURCE_DIR "${YAMLCPP_SOURCE_DIR}" CACHE STRING "yaml-cpp source directory")
endmacro()

# Process the libyaml subdirectory
macro (BuildYamlcpp)
  if (NOT IS_YAMLCPP_ALREADY_BUILT)

    # Make sure YAMLCPP_SOURCE_DIR is set
    EkatSetYamlcppSourceDir()

    set (YAMLCPP_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/yaml-cpp)

    # We don't want testing or any yaml-cpp executable at all
    option (YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
    option (YAML_CPP_BUILD_TESTS "Enable yaml-cpp tests" OFF)
    add_subdirectory (${YAMLCPP_SOURCE_DIR} ${YAMLCPP_BINARY_DIR})

    set (YAMLCPP_INCLUDE_DIRS
       $<BUILD_INTERFACE:${YAMLCPP_SOURCE_DIR}/include>
       $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/yaml-cpp/include>)
    set (YAMLCPP_LIBRARIES yaml-cpp)

    if (EKAT_DISABLE_TPL_WARNINGS)
      include (EkatUtils)
      EkatDisableAllWarning(yaml-cpp)
    endif ()

    # Make sure it is processed only once
    set_property(GLOBAL PROPERTY EKAT_YAMLCPP_BUILT TRUE)
  endif()
endmacro()
