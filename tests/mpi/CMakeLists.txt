include(EkatCreateUnitTest)

# Comm tests
EkatCreateUnitTest(comm comm.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "${EKAT_TEST_MPI_EXTRA_ARGS}"
)
