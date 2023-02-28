macro (IsDebugBuild OUT_VAR_NAME)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" INTERNAL_BUILD_TYPE_CI)
  if ("${INTERNAL_BUILD_TYPE_CI}" STREQUAL "debug")
    set (${OUT_VAR_NAME} TRUE CACHE INTERNAL "")
  else ()
    set (${OUT_VAR_NAME} FALSE CACHE INTERNAL "")
  endif()
endmacro()

function (GetRangeBounds RANGE_SPECS PREFIX)
  list(LENGTH RANGE_SPECS NUM_ARGS)
  if (NUM_ARGS GREATER 3)
    message(FATAL_ERROR "Too many arguments to GetRange: ${RANGE_SPECS}")
  endif()
  if (NUM_ARGS EQUAL 0)
    message(FATAL_ERROR "No arguments to GetRange, RANGE_SPECS:'${RANGE_SPECS}'")
  endif()
  message ("  -> get bounds RANGE SPECS: '${RANGE_SPECS}', NUM_ARGS=${NUM_ARGS}")

  if(NUM_ARGS EQUAL 1)
    list(GET RANGE_SPECS 0 ${PREFIX}_START)
    set (${PREFIX}_END        ${${PREFIX}_START})
    set (${PREFIX}_INCREMENT  1)
  elseif(NUM_ARGS EQUAL 2)
    list(GET RANGE_SPECS 0 ${PREFIX}_START)
    list(GET RANGE_SPECS 1 ${PREFIX}_END)
    set (${PREFIX}_INCREMENT 1)
  else()
    list(GET RANGE_SPECS 0 ${PREFIX}_START)
    list(GET RANGE_SPECS 1 ${PREFIX}_END)
    list(GET RANGE_SPECS 2 ${PREFIX}_INCREMENT)
  endif()

  # Sanity check
  if (${PREFIX}_START      LESS_EQUAL 0 OR
      ${PREFIX}_END        LESS_EQUAL 0 OR
      ${PREFIX}_INCREAMENT LESS_EQUAL 0)
    message ("Error! GetRange requires non-negative start/end/increment.")
    message ("   RANGE_SPECS    : ${RANGE_SPECS}")
    message ("   PREFIX         : ${PREFIX}")
    message ("   RANGE START    : ${${PREFIX}_START}")
    message ("   RANGE END      : ${${PREFIX}_END}")
    message ("   RANGE INCREMENT: ${${PREFIX}_INCREMENT}")
    message (FATAL_ERROR "\nAborting")
  endif()
  if (${PREFIX}_START GREATER ${PREFIX}_END)
    message ("Error! GetRange does not allow 'backward' ranges.")
    message ("   RANGE_SPECS    : ${RANGE_SPECS}")
    message ("   PREFIX         : ${PREFIX}")
    message ("   RANGE START    : ${${PREFIX}_START}")
    message ("   RANGE END      : ${${PREFIX}_END}")
    message ("   RANGE INCREMENT: ${${PREFIX}_INCREMENT}")
    message (FATAL_ERROR "\nAborting")
  endif()

  message ("get bounds, ${PREFIX}_START=${${PREFIX}_START}, ${PREFIX}_END=${${PREFIX}_END}, ${PREFIX}_INCREMENT=${${PREFIX}_INCREMENT}")
  set (${PREFIX}_START      ${${PREFIX}_START}      PARENT_SCOPE)
  set (${PREFIX}_END        ${${PREFIX}_END}        PARENT_SCOPE)
  set (${PREFIX}_INCREMENT  ${${PREFIX}_INCREMENT}  PARENT_SCOPE)

endfunction (GetRangeBounds)

function (GetRange RANGE_SPECS OUTPUT)
  # Parse optional args
  set(options)
  set(args_1v PREPEND)
  set(args_mv)
  cmake_parse_arguments(PARSE "${options}" "${args_1v}" "${args_mv}" ${ARGN})
  CheckMacroArgs(GetRange PARSE "${options}" "${args_1v}" "${args_mv}")

  GetRangeBounds(RANGE_SPECS RANGE) 

  set (RESULT)
  foreach (item IN RANGE ${RANGE_START} ${RANGE_END} ${RANGE_INCREMENT})
    list (APPEND RESULT ${PARSE_PREPEND}${item})
  endforeach()
  set (${OUTPUT} ${RESULT} PARENT_SCOPE)
endfunction ()

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
                      WORKING_DIRECTORY ${EKAT_BINARY_DIR}
                      INPUT_FILE ${CONFIG_FILE_C}
                      OUTPUT_FILE ${EKAT_CONFIGURE_FILE_F90_FILE})

      # do the same for '//...' comments (turn them into '! ...'
      execute_process(COMMAND sed "s;^//;!;g"
                      WORKING_DIRECTORY ${EKAT_BINARY_DIR}
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
  if (${CMAKE_VERSION} VERSION_LESS "3.14.0")
    target_compile_options (${targetName} PRIVATE
      $<$<COMPILE_LANGUAGE:Fortran>:$<$<STREQUAL:"${CMAKE_Fortran_COMPILER_ID}","GNU">:-w> $<$<STREQUAL:"${CMAKE_Fortran_COMPILER_ID}","Intel">: -w>>)
  else ()
    target_compile_options (${targetName} PRIVATE
      $<$<COMPILE_LANGUAGE:Fortran>:$<$<Fortran_COMPILER_ID:GNU>:-w> $<$<Fortran_COMPILER_ID:Intel>: -w>>)
  endif()
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
