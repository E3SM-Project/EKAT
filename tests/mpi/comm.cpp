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
    const float  sum_gauss_f = sum_gauss_i;
    const double sum_gauss_d = sum_gauss_i;

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
    const float  sum_gauss_f = sum_gauss_i;
    const double sum_gauss_d = sum_gauss_i;

    REQUIRE (sum_i==sum_gauss_i);
    REQUIRE (sum_f==sum_gauss_f);
    REQUIRE (sum_d==sum_gauss_d);
  }

  SECTION ("gather") {
    int*    ranks_i = new int   [size];
    float*  ranks_f = new float [size];
    double* ranks_d = new double[size];

    int    rank_i = rank;
    float  rank_f = rank;
    double rank_d = rank;

    comm.all_gather(&rank_i, ranks_i, 1);
    comm.all_gather(&rank_f, ranks_f, 1);
    comm.all_gather(&rank_d, ranks_d, 1);

    for (int i=0; i<size; ++i) {
      REQUIRE (ranks_i [i]==i);
      REQUIRE (ranks_f [i]==i);
      REQUIRE (ranks_d [i]==i);
    }

    delete[] ranks_i;
    delete[] ranks_f;
    delete[] ranks_d;
  }

  SECTION ("split") {
    auto new_comm = comm.split(rank % 2);
    
    const bool am_even = rank%2 == 0;
    const int num_odd  = size/2;
    const int num_even = size%2 + num_odd;

    // Check the new comm has the right size
    REQUIRE ( ((am_even && new_comm.size()==num_even) ||
               (not am_even && new_comm.size()==num_odd)) );

    // Check that the right world ranks ended up in the new comm
    int* ranks = new int[new_comm.size()];

    new_comm.all_gather(&rank,ranks,1);

    for (int i=0; i<new_comm.size(); ++i) {
      if (am_even) {
        REQUIRE ( ranks[i] == 2*i );
      } else {
        REQUIRE ( ranks[i] == (2*i+1) );
      }
    }

    delete[] ranks;
  }
}

} // anonymous namespace
