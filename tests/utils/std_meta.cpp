#include <catch2/catch.hpp>

#include "ekat/std_meta/ekat_std_any.hpp"

#include <vector>

TEST_CASE ("any") {
  using namespace ekat;

  any a,b;

  std::vector<int> u = {1,2};

  a.reset<std::vector<int>>(u);
  b.reset(u);

  REQUIRE (any_cast<std::vector<int>>(a)==any_cast<std::vector<int>>(b));
  REQUIRE (any_cast<std::vector<int>>(a)==u);
  REQUIRE_THROWS (any_cast<std::vector<double>>(a));

  REQUIRE (a.isType<std::vector<int>>());
  REQUIRE (not a.isType<int>());

  any c (u);
  REQUIRE (any_cast<std::vector<int>>(a)==any_cast<std::vector<int>>(c));
}
