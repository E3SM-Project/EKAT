#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "ekat_test_config.h"
#include "tridiag_tests.hpp"
#include "ekat_kokkos_session.hpp"

int main (int argc, char **argv) {
  int num_failed = 0;
  ekat::initialize_kokkos_session(argc, argv); {
    if (argc > 1) {
      // Performance test.
      ekat::test::perf::Input in;
      const auto stat = in.parse(argc, argv);
      if (stat)
        ekat::test::perf::run<Real>(in);
      else
        return -1;
    } else {
      // Correctness tests.
      num_failed = Catch::Session().run(argc, argv);
    }
  } ekat::finalize_kokkos_session();
  return num_failed != 0 ? 1 : 0;
}
