include(EkatCreateUnitTest)

# Test units framework
EkatCreateUnitTest(logger logger_tests.cpp LIBS ekat)

EkatCreateUnitTest(mpilogger mpi_logger_tests.cpp
  LIBS ekat
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_EXTRA_ARGS "--oversubscribe"
)
