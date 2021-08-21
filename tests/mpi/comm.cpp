#include <catch2/catch.hpp>
#include "ekat/mpi/ekat_comm.hpp"

namespace {

TEST_CASE ("ekat_comm","") {
  using namespace ekat;

  Comm comm(MPI_COMM_WORLD);
  const int rank = comm.rank();
  const int size = comm.size();

  SECTION ("scan") {
    int    val_i = rank;
    float  val_f = rank;
    double val_d = rank;

    int    sum_i;
    float  sum_f;
    double sum_d;

    comm.scan(&val_i,&sum_i,1,MPI_SUM);
    comm.scan(&val_f,&sum_f,1,MPI_SUM);
    comm.scan(&val_d,&sum_d,1,MPI_SUM);

    const int    sum_gauss_i = rank*(rank+1)/2;
    const float  sum_gauss_f = rank*(rank+1)/2;
    const double sum_gauss_d = rank*(rank+1)/2;

    REQUIRE(sum_i==sum_gauss_i);
    REQUIRE(sum_f==sum_gauss_f);
    REQUIRE(sum_d==sum_gauss_d);
  }

  SECTION ("broadcast") {
    int*    ints    = new int   [size];
    float*  floats  = new float [size];
    double* doubles = new double[size];

    ints   [rank] = -rank;
    floats [rank] = -rank;
    doubles[rank] = -rank;

    for (int i=0; i<size; ++i) {
      comm.broadcast(&ints   [i],1,i);
      comm.broadcast(&floats [i],1,i);
      comm.broadcast(&doubles[i],1,i);

      REQUIRE (ints   [i]==-i);
      REQUIRE (floats [i]==-i);
      REQUIRE (doubles[i]==-i);
    }

    delete[] ints;
    delete[] floats;
    delete[] doubles;
  }

  SECTION ("reduce") {
    int    val_i = rank;
    float  val_f = rank;
    double val_d = rank;

    int    sum_i;
    float  sum_f;
    double sum_d;

    comm.all_reduce(&val_i,&sum_i,1,MPI_SUM);
    comm.all_reduce(&val_f,&sum_f,1,MPI_SUM);
    comm.all_reduce(&val_d,&sum_d,1,MPI_SUM);

    const int n = size - 1;

    const int    sum_gauss_i = n*(n+1)/2;
    const float  sum_gauss_f = n*(n+1)/2;
    const double sum_gauss_d = n*(n+1)/2;

    REQUIRE (sum_i==sum_gauss_i);
    REQUIRE (sum_f==sum_gauss_f);
    REQUIRE (sum_d==sum_gauss_d);
  }
}

} // anonymous namespace
