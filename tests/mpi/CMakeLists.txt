include(EkatCreateUnitTest)

# Comm tests
EkatCreateUnitTest(comm comm.cpp
  LIBS MpiComm
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
)
