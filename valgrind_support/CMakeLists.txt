include(EkatCreateUnitTest)

# Path to valgrind suppression file. If none is provided, EKAT will generate one
# for you. The generated ones are OK but there's an element of nondeterminism
# in some of the valgrind interactions with some of our TPLs (like MPI), so
# a persistent, maintained suppression file is best for consistency.
#
# If new valgrind errors emerge that you want to suppress, run the error text
# through gen_sup.sh and add the output to the stored suppression file.
set(DEFAULT_SUPPRESSION_FILE ${CMAKE_BINARY_DIR}/ekat_generated_valgrind.supp)
set(EKAT_VALGRIND_SUPPRESSION_FILE ${DEFAULT_SUPPRESSION_FILE} CACHE FILEPATH "Use this valgrind suppression file if valgrind is enabled.")

if (EKAT_VALGRIND_SUPPRESSION_FILE STREQUAL DEFAULT_SUPPRESSION_FILE)

  EkatCreateUnitTestExec(catch_test catch_test.cpp)

  # When passing the invoke string to add_custom_command, if there's a space in one of
  # the args, CMake will add "\" in front of it, to avoid it being parsed as a list separator.
  # Unfortunately, that's a pb in bash, since \ means something. Therefore, we ensure that
  # there are NO SPACES in the invoke command, storing each token as a list item.
  set (invokeValg valgrind --gen-suppressions=all)
  if (EKAT_ENABLE_MPI)
    list (PREPEND invokeValg ${EKAT_MPIRUN_EXE} ${EKAT_MPI_NP_FLAG} 2 ${EKAT_MPI_EXTRA_ARGS})
    list (TRANSFORM invokeValg REPLACE " " ";")
  endif()
  list (APPEND invokeValg ./catch_test --args --one --two --three=3 --other=4,5)

  set (VALGRIND_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ekat_valg_output.txt)
  message ("invoke: ${invokeValg}")

  # RUn valgrind and generate output file
  add_custom_command(
    OUTPUT ${VALGRIND_OUTPUT}
    COMMAND ${invokeValg} > ${VALGRIND_OUTPUT} 2>&1
    DEPENDS catch_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating valgrind output file..."
  )

  # Parse output to generate suppressions file
  add_custom_command(
    OUTPUT ${EKAT_VALGRIND_SUPPRESSION_FILE}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh ${VALGRIND_OUTPUT} > ${EKAT_VALGRIND_SUPPRESSION_FILE}
    DEPENDS ${VALGRIND_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating valgrind suppression file ${EKAT_VALGRIND_SUPPRESSION_FILE}..."
  )

  # Add the target
  add_custom_target(valgrind_supp ALL DEPENDS ${EKAT_VALGRIND_SUPPRESSION_FILE})
endif()
