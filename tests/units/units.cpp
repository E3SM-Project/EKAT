#include <catch2/catch.hpp>

#include "ekat/util/ekat_units.hpp"

#include <iostream>

TEST_CASE("units_framework", "") {
  using namespace ekat;
  using namespace ekat::units;

  SECTION ("rational_constant") {
    constexpr RationalConstant quarter(1,4);
    constexpr RationalConstant half(1,2);
    constexpr RationalConstant one(1);
    constexpr RationalConstant two(2,1);

    // Verify operations
    REQUIRE(half*two == one);
    REQUIRE(half+half == one);
    REQUIRE(one/two == half);
    REQUIRE(half*half == quarter);
    REQUIRE(pow(half,2)==quarter);
#if defined(EKAT_CONSTEXPR_ASSERT) && !defined(NDEBUG)
    const RationalConstant zero(0);
    REQUIRE_THROWS(one/zero);
    REQUIRE_THROWS(pow(zero,zero));
    REQUIRE_THROWS(pow(-one,half));
#endif
  }

  SECTION ("scaling_factor") {
    constexpr RationalConstant one = RationalConstant::one();
    constexpr RationalConstant third = 1/(3*one);
    constexpr RationalConstant four_thirds = 4*third;
    constexpr RationalConstant three_halves = 3/(2*one);
    constexpr RationalConstant two = 2*one;
    constexpr ScalingFactor root2 (2,RationalConstant{1,2});

    // Verify operations
    REQUIRE( root2*root2 == two );
    REQUIRE( sqrt(two) == root2 );
    REQUIRE( pow(three_halves,2)*pow(four_thirds,3) == 16*one/3);

    // Verify printing
    REQUIRE(to_string(root2)=="2^1/2");
    REQUIRE(to_string(root2,Format::Float)=="2^0.5");

#if defined(EKAT_CONSTEXPR_ASSERT) && !defined(NDEBUG)
    const RationalConstant three(3);
    REQUIRE_THROWS(sqrt(-three));
#endif
  }

  SECTION ("units") {
    using namespace prefixes;

    constexpr RationalConstant one = RationalConstant::one();
    const auto km = kilo*m;
    const auto kPa = kilo*Pa;

    Units nondim (ScalingFactor(1));
    Units milliJ = milli*N*m;
    Units mix_ratio = kg/kg;
    mix_ratio.set_string("kg/kg");

    // Verify operations
    REQUIRE (milliJ == kPa*pow(m,3)/mega);
    REQUIRE (m/s*day/km == Units(one*86400/1000));
    REQUIRE (pow(sqrt(m),2)==m);

    // Verify printing
    REQUIRE (to_string(nondim)=="1");
    REQUIRE (to_string(milliJ)=="0.001 m^2 s^-2 kg");

    // Verify changing the string works and does not affect the to_string function
    REQUIRE (mix_ratio==nondim);
    REQUIRE (to_string(mix_ratio)=="1");
    REQUIRE (mix_ratio.get_string()=="kg/kg");
  }

  SECTION ("issue-52") {
    auto one_over_mol = ScalingFactor::one() / mol;
    auto mol_inverse = pow(mol, -1);

    REQUIRE (one_over_mol == mol_inverse);

    auto mol_mol = mol * mol;
    auto mol_2 = pow(mol,2);

    REQUIRE (mol_mol == mol_2);
  }
}
