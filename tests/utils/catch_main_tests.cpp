#include <catch2/catch.hpp>

#include "ekat/util/ekat_test_utils.hpp"

namespace {

TEST_CASE ("flags_and_params") {
  auto& ts = ekat::TestSession::get();

  const auto& f = ts.flags;
  std::cout << "flags:";
  for (const auto& it : f) {
    std::cout << " " << it.first;
    // All flags added should be added as true
    REQUIRE (it.second);
  }
  std::cout << "\n";

  const auto& p = ts.params;
  std::cout << "params:";
  for (const auto& it : p) {
    std::cout << " " << it.first << "=" << it.second;
  }
  std::cout << "\n";
}

}
