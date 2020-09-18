#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "tridiag_tests.hpp"
#include "ekat/ekat_session.hpp"
#include "ekat_test_config.h"

int main (int argc, char **argv) {
  int num_failed = 0;
  ekat::initialize_ekat_session(argc, argv); {
    if (argc > 1) {
      // Performance test.
      ekat::test::perf::Input in;
      const auto stat = in.parse(argc, argv);
      if (stat) ekat::test::perf::run<Real>(in);
    } else {
      // Correctness tests.
      num_failed = Catch::Session().run(argc, argv);
    }
  } ekat::finalize_ekat_session();
  return num_failed != 0 ? 1 : 0;
}
