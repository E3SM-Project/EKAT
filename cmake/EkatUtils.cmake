# Useful function, that grabs all the cmake vars visible in this scope
# and prints them to screen. One can pass REGEX <regex> to filter
# which vars to print
function(dump_cmake_variables)
  set (opts)
  set (args1v REGEX)
  set (argsMv)
  cmake_parse_arguments (dcv "${opts}" "${args1v}" "${argsMv}" ${ARGN})

  get_cmake_property(dcv_var_names VARIABLES)

  foreach (var_name IN LISTS dcv_var_names)
    if (dcv_REGEX)
      string(REGEX MATCH ${dcv_REGEX} MATCHES ${var_name})
      if (MATCHES)
        message(STATUS "${var_name}=${${var_name}}")
      endif()
    else ()
      message(STATUS "${var_name}=${${var_name}}")
    endif ()
  endforeach()
endfunction()

macro (IsDebugBuild OUT_VAR_NAME)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" INTERNAL_BUILD_TYPE_CI)
  if ("${INTERNAL_BUILD_TYPE_CI}" STREQUAL "debug")
    set (${OUT_VAR_NAME} TRUE CACHE INTERNAL "")
  else ()
    set (${OUT_VAR_NAME} FALSE CACHE INTERNAL "")
  endif()
endmacro()

macro (CheckMacroArgs macroName parsePrefix validOptions validOneValueArgs validMultiValueArgs)
  if (${parsePrefix}_UNPARSED_ARGUMENTS)
    message (AUTHOR_WARNING
             "Warning: the following arguments to macro ${macroName} were not recognized:\n"
             "   ${${parsePrefix}_UNPARSED_ARGUMENTS}\n"
             " Here's a list of valid arguments:\n"
             "   options: ${validOptions}\n"
             "   oneValueArgs: ${validOneValueArgs}\n"
             "   multiValueArgs: ${validMultiValueArgs}\n")
  endif ()

  if (${parsePrefix}_KEYWORDS_MISSING_VALUES)
    message (AUTHOR_WARNING
             "Warning: the following keywords in macro ${macroName} were used, but no argument was provided:\n"
             "   ${${parsePrefix}_KEYWORDS_MISSING_VALUES}\n")
  endif ()
endmacro ()

macro (EkatConfigFile CONFIG_FILE_IN CONFIG_FILE_C)
  set(options AT_ONLY)
  set(oneValueArgs F90_FILE)
  set(multiValueArgs)

  cmake_parse_arguments(EKAT_CONFIGURE_FILE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  CheckMacroArgs(EkatConfigFile EKAT_CONFIGURE_FILE "${options}" "${oneValueArgs}" "${multiValueArgs}")

  # Generate temporary config file
  if (EKAT_CONFIGURE_FILE_AT_ONLY)
    configure_file (${CONFIG_FILE_IN} ${CONFIG_FILE_C}.tmp @ONLY)
  else()
    configure_file (${CONFIG_FILE_IN} ${CONFIG_FILE_C}.tmp)
  endif()

  # Assume by default that config file is out of date
  set (OUT_OF_DATE TRUE)

  # If config file in binary dir exists, we check whether the new one would be different
  if (EXISTS ${CONFIG_FILE_C})

    # We rely on FILE macro rather than running diff, since it is
    # more portable (guaranteed to work regardless of underlying system)
    file (READ ${CONFIG_FILE_C} CONFIG_FILE_C_STR)
    file (READ ${CONFIG_FILE_C}.tmp CONFIG_FILE_C_TMP_STR)

    if ("${CONFIG_FILE_C_STR}" STREQUAL "${CONFIG_FILE_C_TMP_STR}")
      # config file was present and appears unchanged
      set (OUT_OF_DATE FALSE)
    endif()

    FILE (REMOVE ${CONFIG_FILE_C}.tmp)
  endif ()

  # If out of date (either missing or different), adjust
  if (OUT_OF_DATE)

    # Run the configure macro
    configure_file (${CONFIG_FILE_IN} ${CONFIG_FILE_C})

    if (EKAT_CONFIGURE_FILE_F90_FILE)
      # run sed to change '/*...*/' comments into '!/*...*/'
      execute_process(COMMAND sed "s;^/;!/;g"
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      INPUT_FILE ${CONFIG_FILE_C}
                      OUTPUT_FILE ${EKAT_CONFIGURE_FILE_F90_FILE})

      # do the same for '//...' comments (turn them into '! ...'
      execute_process(COMMAND sed "s;^//;!;g"
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      INPUT_FILE ${CONFIG_FILE_C}
                      OUTPUT_FILE ${EKAT_CONFIGURE_FILE_F90_FILE})
    endif()
  endif()

endmacro (EkatConfigFile)

macro (EkatDisableAllWarning targetName)

  if (NOT TARGET ${targetName})
    message (FATAL_ERROR "Error! Cannot disable warnings for target ${targetName}; it is not built by this project.")
  endif ()

  # Better let the user know, just in case he wasn't expecting this.
  message (STATUS "Disabling all warnings for target ${targetName}")

  # Add flags to ignore warnings to the target, for all Ekat-supported languages (C, CXX, Fortran)
  # Make the flag compiler-dependent. Notice that only one of the $<$<C_COMPILER_ID:blah>: "blahblah">
  # will expand to anything at all, so this is ok.
  # Note: even if a compiler collection (usually) has the same flag for all languages, we still
  #       add the flag separately for each langauge, since the user MAY be using different compilers
  #       for different langauges (e.g., icpc and gfortran).
  # TODO: if we upgrade required cmake version to 3.16, we can use the more compact generator expression:
  #         target_compile_options (${targetName} PRIVATE $<$<COMPILE_LANG_AND_ID:C,GNU,Intel>:-w>)
  target_compile_options (${targetName} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:$<$<C_COMPILER_ID:GNU>:-w> $<$<C_COMPILER_ID:Intel>: -w>>)
  target_compile_options (${targetName} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-w -fcompare-debug-second> $<$<CXX_COMPILER_ID:Intel>: -w>>)
  target_compile_options (${targetName} PRIVATE
    $<$<COMPILE_LANGUAGE:Fortran>:$<$<Fortran_COMPILER_ID:GNU>:-w> $<$<Fortran_COMPILER_ID:Intel>: -w>>)
endmacro (EkatDisableAllWarning)

function(separate_cut_arguments prefix options oneValueArgs multiValueArgs return_varname)
  set(result)
  foreach(item IN LISTS options)
    if (${prefix}_${item})
      list(APPEND result ${item})
    endif()
  endforeach()

  foreach(item IN LISTS oneValueArgs multiValueArgs)
    if (${prefix}_${item})
      list(APPEND result ${item} ${${prefix}_${item}})
    endif()
  endforeach()

  set(${return_varname} ${result} PARENT_SCOPE)
endfunction(separate_cut_arguments)

# Utility to avoid race conditions in FetchContent_MakeAvailable
# when 2+ builds are configuring at the same time. Only the 1st one
# will actually populate (i.e. clone) the TPL folder, and ONLY if
# the folder doesn't exist or contains a different version of the code
function (ekat_fetch_content NAME)
  # 0.b Check if we already parsed the TPL (if so, we're done)
  get_property(already_added GLOBAL PROPERTY EKAT_TPL_${NAME}_ADDED)
  if (already_added)
    get_property(first_caller GLOBAL PROPERTY EKAT_TPL_${NAME}_CALLER)
    message(AUTHOR_WARNING
      "  ${NAME}: This TPL has already been added to the build tree.\n"
      "  Subsequent calls to ekat_fetch_content(${NAME}) will NOT reconfigure the TPL.\n"
      "  First added from: ${first_caller}\n"
      "  If you need to change TPL options, do it at the first call site."
    )
    return()
  endif()

  # 0.b Support the standard CMake override: -DFETCHCONTENT_SOURCE_DIR_<NAME>=/path/to/src
  string(TOUPPER "${NAME}" NAME_UPPER)
  string(REPLACE "-" "_" NAME_SANITIZED "${NAME_UPPER}")
  set(OVERRIDE_VAR "FETCHCONTENT_SOURCE_DIR_${NAME_SANITIZED}")
  if (DEFINED ${OVERRIDE_VAR})
    message(STATUS "  ${NAME}: Using source override from ${${OVERRIDE_VAR}}")
    # Mark as added and jump straight to the finish
    add_subdirectory("${${OVERRIDE_VAR}}" "${CMAKE_BINARY_DIR}/externals/${NAME}")
    set_property(GLOBAL PROPERTY EKAT_TPL_${NAME}_ADDED TRUE)
    set_property(GLOBAL PROPERTY EKAT_TPL_${NAME}_CALLER "${CMAKE_CURRENT_LIST_FILE}")
    return()
  endif()

  # 1. Define the expected arguments
  set(options "")
  set(oneValueArgs GIT_REPOSITORY GIT_TAG SOURCE_DIR)
  set(multiValueArgs "")
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # 2. Setup paths and lock (a hidden file in the source dir parent folder)
  get_filename_component(ABS_SOURCE_DIR "${ARG_SOURCE_DIR}" ABSOLUTE)
  get_filename_component(PARENT_DIR "${ABS_SOURCE_DIR}" DIRECTORY)
  set(LOCK_FILE "${PARENT_DIR}/.${NAME}.lock")

  # 3. Serialized section: lock the shared source area to verify/populate
  file(LOCK "${LOCK_FILE}" GUARD FUNCTION TIMEOUT 600)

  # 4. Check if we need to populate the TPL source dir
  set (POPULATE TRUE)
  if (EXISTS "${ABS_SOURCE_DIR}/.git/index")
    # Ask Git for the current HEAD commit hash
    execute_process(
      COMMAND git rev-parse HEAD
      WORKING_DIRECTORY ${ABS_SOURCE_DIR}
      OUTPUT_VARIABLE ON_DISK_SHA
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if ("${ON_DISK_SHA}" STREQUAL "${ARG_GIT_TAG}")
      message (STATUS "  ${NAME} source dir already populated with correct version. Skipping populate...")
      set (POPULATE FALSE)
    else()
      message (STATUS "  ${NAME} source dir already populated but with incorrect version. Calling populate...\n"
                      "    Found version: ${ON_DISK_SHA}"
                      "    Expected version: ${ARG_GIT_TAG}")
    endif()
  endif()

  # 5. Populate (if needed)
  if (POPULATE)
    # Use FetchContent internally for the one build that 'wins' the lock
    include(FetchContent)
    FetchContent_Declare(${NAME}
        GIT_REPOSITORY ${ARG_GIT_REPOSITORY}
        GIT_TAG        ${ARG_GIT_TAG}
        SOURCE_DIR     ${ABS_SOURCE_DIR}
    )

    # This performs the actual git clone/checkout
    FetchContent_Populate(${NAME})
  endif ()

  # We can finally release the lock
  file(LOCK "${LOCK_FILE}" RELEASE)

  # 6. Parse subfolder, and set global property (so we don't re-add it by mistake)
  add_subdirectory("${ABS_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/externals/${NAME}")
  set_property(GLOBAL PROPERTY EKAT_TPL_${NAME}_ADDED TRUE)
  set_property(GLOBAL PROPERTY EKAT_TPL_${NAME}_CALLER "${CMAKE_CURRENT_LIST_FILE}")
endfunction()
