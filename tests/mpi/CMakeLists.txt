include(EkatCreateUnitTest)

# Comm tests
EkatCreateUnitTest(comm comm.cpp
  LIBS ekat
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
)
