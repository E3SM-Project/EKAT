#include <catch2/catch.hpp>
#include "ekat/mpi/ekat_comm.hpp"

namespace {

TEST_CASE ("ekat_comm","") {
  using namespace ekat;

  Comm comm(MPI_COMM_WORLD);

  int r_p1 = comm.rank()+1;
  int sum;
  comm.scan_sum(&r_p1,&sum,1);

  int gauss_formula = r_p1*(r_p1+1)/2;

  REQUIRE(sum==gauss_formula);

}

} // anonymous namespace
