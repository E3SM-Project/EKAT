include(EkatCreateUnitTest)

if (NOT EKAT_VALGRIND_SUPPRESSION_FILE)
  add_executable(serial_hw serial_hw.cxx)
  add_executable(mpi_hw mpi_hw.cxx)

  EkatCreateUnitTestExec(mpi_valg_catch_test mpi_catch_test.cpp)

  # We know this var was not set so it's safe to FORCE set it.
  set(EKAT_VALGRIND_SUPPRESSION_FILE ${CMAKE_BINARY_DIR}/ekat_gen_valgrind.supp CACHE FILEPATH "" FORCE)

  if (EKAT_ENABLE_MPI)
    add_custom_target(mpi_sup ALL
      COMMAND ${EKAT_MPIRUN_EXE} ${EKAT_MPI_NP_FLAG} 2 ${EKAT_MPI_EXTRA_ARGS} valgrind --gen-suppressions=all ./mpi_hw 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${EKAT_VALGRIND_SUPPRESSION_FILE}
      COMMAND ${EKAT_MPIRUN_EXE} ${EKAT_MPI_NP_FLAG} 2 ${EKAT_MPI_EXTRA_ARGS} valgrind --gen-suppressions=all ./mpi_valg_catch_test 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh >> ${EKAT_VALGRIND_SUPPRESSION_FILE}
      DEPENDS mpi_hw
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
  else()
    add_custom_target(serial_sup ALL
      COMMAND valgrind --gen-suppressions=all ./serial_hw 2>&1 | ${CMAKE_CURRENT_SOURCE_DIR}/gen_sup.sh > ${EKAT_VALGRIND_SUPPRESSION_FILE}
      DEPENDS serial_hw
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
  endif()
endif()
