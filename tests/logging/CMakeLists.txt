include(EkatCreateUnitTest)

# Test basic logger capabilities
EkatCreateUnitTest(serial_file_log serial_file_log_tests.cpp LIBS ekat)

EkatCreateUnitTest(mpi_file_log_tests mpi_file_log_tests.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
  PROPERTIES FAIL_REGULAR_EXPRESSION "rank1"
)

# Console-only output. One logger should print "all_ranks.$size.$rank"
# for all ranks. The other should print only "root_rank.$size.0"
set (pass "all_ranks.${EKAT_TEST_MAX_RANKS}.${RANK}")
set (fail "QWERTY") # Not in the output anyways

foreach(RANK RANGE 1 ${EKAT_TEST_MAX_RANKS})
  string(APPEND pass "|all_ranks.${EKAT_TEST_MAX_RANKS}.${RANK}")
  if (RANK GREATER 1)
    string(APPEND fail "|root_rank.${EKAT_TEST_MAX_RANKS}.${RANK}")
  endif()
endforeach()

EkatCreateUnitTest(console_only_log console_only_log_tests.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
  PROPERTIES PASS_REGULAR_EXPRESSION ${pass}
  PROPERTIES FAIL_REGULAR_EXPRESSION ${fail}
)
