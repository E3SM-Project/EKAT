#Try to find the package
message (STATUS "looking for yaml ... ")
if (NOT "$ENV{YAML_CPP_ROOT}" STREQUAL "")
  set (YAML_CPP_ROOT $ENV{YAML_CPP_ROOT})
endif()

if (YAML_CPP_ROOT)
  # Look for headers and lib
  find_path(YAML_CPP_INCLUDE_DIR
    NAMES yaml.h
    PATHS ${YAML_CPP_ROOT}
    PATH_SUFFIXES include include/yaml-cpp)
  find_library(YAML_CPP_LIB
    NAMES yaml-cpp yaml-cppd
    PATHS ${YAML_CPP_ROOT}
    PATH_SUFFIXES lib lib64)
  if (NOT YAML_CPP_LIB STREQUAL "YAML_CPP_LIB-NOTFOUND" AND
      NOT YAML_CPP_INCLUDE_DIR STREQUAL "YAML_CPP_INCLUDE_DIR-NOTFOUND")
    set (YAML_CPP_FOUND TRUE)
  endif()
endif()

if (YAML_CPP_FOUND)
  message (STATUS "looking for yaml ... FOUND")
  message (STATUS "  YAML_CPP_LIB: ${YAML_CPP_LIB}")
  message (STATUS "  YAML_CPP_INCLUDE DIR: ${YAML_CPP_INCLUDE_DIR}")
  add_library(yaml-cpp INTERFACE)
  target_link_libraries (yaml-cpp INTERFACE ${YAML_CPP_LIB})
  target_include_directories (yaml-cpp INTERFACE ${YAML_CPP_INCLUDE_DIR})
else()
  message (STATUS "looking for yaml ... NOT FOUND")
  message (STATUS "  -> Downloading and building locally in ${EKAT_BINARY_DIR}/tpls")
  include (FetchContent)

  # Fetch and populate the external project
  set (FETCHCONTENT_BASE_DIR ${EKAT_BINARY_DIR}/tpls)

  FetchContent_Declare (
    yaml-cpp
    GIT_REPOSITORY git@github.com:snlcomputation/yaml-cpp
    GIT_TAG        336f763e63e15e43877a37b2d8c9c923e84dcd16
    OVERRIDE_FIND_PACKAGE
  )

  # Set options for yamlcpp before adding the subdirectory
  option (YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
  option (YAML_CPP_BUILD_TESTS "Enable yaml-cpp tests" OFF)

  message (STATUS " *** Begin of yaml-cpp configuration ***")
  FetchContent_MakeAvailable (yaml-cpp)
  message (STATUS " ***  End of yaml-cpp configuration  ***")

  if (EKAT_DISABLE_TPL_WARNINGS)
    include (EkatUtils)
    EkatDisableAllWarning(yaml-cpp)
  endif ()
endif()
