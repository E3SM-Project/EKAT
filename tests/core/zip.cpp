#include <catch2/catch.hpp>

#include "ekat_zip.hpp"

#include <vector>
#include <list>
#include <string>

namespace ekat {

TEST_CASE ("ekat_comm","") {
  std::vector<int> v1 = {1,2,3};
  std::vector<int> v2 = {-1,-2,-3};
  std::vector<int> v3 = {-1};
  std::list<std::string> l1 = {"1", "2", "3"};

  REQUIRE_THROWS(zip(v1,v3));

  for (auto [pos,neg] : zip(v1,v2)) {
    REQUIRE (pos==(-neg));
  }
  for (const auto& [i,s] : zip(v1,l1)) {
    REQUIRE (std::to_string(i)==s);
  }
}

} // namespace ekat

