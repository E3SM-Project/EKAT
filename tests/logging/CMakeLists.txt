include(EkatCreateUnitTest)

# Test units framework
EkatCreateUnitTest(logger logger_tests.cpp LIBS ekat)

EkatCreateUnitTest(mpi_log_all mpi_log_all_ranks.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
)

EkatCreateUnitTest(mpi_log_rank0 mpi_log_rank0_only.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
  PROPERTIES FAIL_REGULAR_EXPRESSION "rank1"
)

EkatCreateUnitTest(multilog multiple_log_example.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
  PROPERTIES FAIL_REGULAR_EXPRESSION "rank1"
)
