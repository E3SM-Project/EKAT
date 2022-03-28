include(EkatCreateUnitTest)

# Test basic logger capabilities
EkatCreateUnitTest(logger logger_tests.cpp LIBS ekat)

EkatCreateUnitTest(mpi_logging mpi_log_tests.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
  PROPERTIES FAIL_REGULAR_EXPRESSION "rank1"
)
