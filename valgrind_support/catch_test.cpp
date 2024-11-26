#include <catch2/catch.hpp>
#include "ekat/util/ekat_test_utils.hpp"
#include "ekat/util/ekat_string_utils.hpp"
#include <iostream>

#include "ekat/mpi/ekat_comm.hpp"

ekat::Comm get_comm()
{
#ifdef EKAT_ENABLE_MPI
  return ekat::Comm(MPI_COMM_WORLD);
#else
  return ekat::Comm();
#endif
}

namespace {

TEST_CASE("catch_dummy") {
  auto& ts = ekat::TestSession::get();

  std::cout << "test flags:\n";
  for (const auto& it : ts.flags) {
    std::cout << " " << it.first << ": " << (it.second ? "true" : "false")<< "\n";
  }
  std::cout << "test params:\n";
  for (const auto& it : ts.params) {
    std::cout << " " << it.first << ": " << it.second << "\n";
  }
  std::cout << "test vec params:\n";
  for (const auto& it : ts.vec_params) {
    std::cout << " " << it.first << ": " << ekat::join(it.second,",") << "\n";
  }
  REQUIRE (true);

  auto comm = get_comm();
  comm.barrier();
  int i = ts.flags.size();
  comm.broadcast(&i,1,0);
}

} // anonymous namespace
