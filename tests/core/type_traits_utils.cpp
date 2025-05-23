#include <catch2/catch.hpp>

#include "ekat_type_traits.hpp"

namespace {

TEST_CASE("precision", "util") {
  CHECK_FALSE(ekat::is_single_precision<double>::value);
  CHECK(ekat::is_single_precision<float>::value);
}

TEST_CASE("type_traits", "") {
  using namespace ekat;
  REQUIRE(std::is_same<ekat::ValueType<double**&>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double*[3]>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double[2][3]>::type,double>::value);

  // Check meta-util to get rank and dynamic rank of a raw MD array
  REQUIRE(ekat::GetRanks<double[2][3]>::rank==2);
  REQUIRE(ekat::GetRanks<double[2][3]>::rank_dynamic==0);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank==3);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank_dynamic==1);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank==4);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank_dynamic==2);
}

} // empty namespace
