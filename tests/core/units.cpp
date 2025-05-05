#include <catch2/catch.hpp>

#include "ekat_units.hpp"

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
#ifndef NDEBUG
    constexpr RationalConstant zero(0);
    // NOTE: even though one/zero CAN be evaluated at compile time,
    // Catch2's impl will defer evaluation of expression until runtime...somehow.
    REQUIRE_THROWS(one/zero);
    REQUIRE_THROWS(pow(zero,zero));
    REQUIRE_THROWS(pow(-one,half));
#endif

    REQUIRE ((-half).to_string()==std::string("-1/2"));
    REQUIRE ((-half).to_string(Format::Float)==std::string("-0.5"));
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
    REQUIRE(root2.to_string()=="2^(1/2)");
    REQUIRE(root2.to_string(Format::Float)=="2^0.5");

#ifndef NDEBUG
    const RationalConstant three(3);
    REQUIRE_THROWS(sqrt(-three));
#endif
  }

  SECTION ("units") {
    using namespace prefixes;

    constexpr RationalConstant one = RationalConstant::one();
    const auto km = kilo*m;
    const auto kPa = kilo*Pa;

    constexpr Units nondim (ScalingFactor(1));
    constexpr Units mJ = milli*J;
    constexpr Units mix_ratio = kg/kg;

    // Verify operations
    REQUIRE (mJ == kPa*pow(m,3)/mega);
    REQUIRE (m/s*day/km == Units(one*86400/1000));
    REQUIRE (pow(sqrt(m),2)==m);

    // Verify all printing properties
    REQUIRE (nondim.to_string()=="1");
    REQUIRE (mJ.to_string()=="mJ");
    REQUIRE ((J/1000).to_string()=="J/1000");
    REQUIRE (mJ.get_si_string()=="1/1000 m^2 s^-2 kg");

    REQUIRE (mix_ratio==nondim);
    REQUIRE (mix_ratio.get_si_string()=="1");
    REQUIRE (mix_ratio.to_string()=="kg/kg");
    REQUIRE ((m*mix_ratio).to_string()=="m*(kg/kg)");
    REQUIRE ((m*mix_ratio).get_si_string()=="m");

    constexpr Units uJ = micro*J;
    constexpr Units ug = micro*g;
    constexpr Units mbar = milli*bar;

    REQUIRE (uJ.to_string()=="uJ");
    REQUIRE ((ug/kg).to_string()=="ug/kg");
    REQUIRE ((mbar/h).to_string()=="mbar/h");

    REQUIRE ((mbar/h).get_si_string()=="1/36 m^-1 s^-3 kg");
    REQUIRE ((ug/kg).get_si_string()=="1/1000000000");

    REQUIRE ((kilo*(ug/kg)).to_string()=="(10^3)*(ug/kg)");
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
