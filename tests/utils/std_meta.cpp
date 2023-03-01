#include <catch2/catch.hpp>

#include "ekat/std_meta/ekat_std_any.hpp"

#include <vector>

TEST_CASE ("any") {

  ekat::any a,b;

  std::vector<int> u = {1,2};

  a.reset<std::vector<int>>(u);
  b.reset(u);

  REQUIRE (ekat::any_cast<std::vector<int>>(a)==ekat::any_cast<std::vector<int>>(b));
  REQUIRE (ekat::any_cast<std::vector<int>>(a)==u);
  REQUIRE_THROWS (ekat::any_cast<std::vector<double>>(a));

  REQUIRE (a.isType<std::vector<int>>());
  REQUIRE (not a.isType<int>());

  ekat::any c (u);
  REQUIRE (ekat::any_cast<std::vector<int>>(a)==ekat::any_cast<std::vector<int>>(c));
}
