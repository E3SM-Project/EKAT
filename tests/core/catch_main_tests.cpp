#include <catch2/catch.hpp>

#include "ekat_test_utils.hpp"
#include "ekat_string_utils.hpp"

namespace {

TEST_CASE ("flags_and_params") {
  auto& ts = ekat::TestSession::get();

  const auto& f = ts.flags;
  std::cout << "flags:";
  for (const auto& it : f) {
    if (it.second) {
      std::cout << " " << it.first;
    }
  }

  const auto& p = ts.params;
  std::cout << ", params:";
  for (const auto& it : p) {
    std::cout << " " << it.first << "=" << it.second;
  }

  const auto& vp = ts.vec_params;
  std::cout << ", vec_params:";
  for (const auto& it : vp) {
    std::cout << " " << it.first << "=" << ekat::join(it.second,",");
  }
  std::cout << "\n";
}

}
