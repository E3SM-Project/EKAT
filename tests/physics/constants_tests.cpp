#include <catch2/catch.hpp>
#include "ekat/physics/constants.hpp"
#include <cmath>

using namespace ekat;
using namespace ekat::physics;

TEST_CASE("physics_constants", "[physics]") {

  using constants = Constants<double>;

  REQUIRE(4*std::atan(1) == Approx(constants::pi));
  REQUIRE(6*constants::pi_sixth == Approx(constants::pi));
  REQUIRE(constants::avogadro == Approx(6.022e23).epsilon(0.01));
  REQUIRE(constants::r_gas == Approx(8.31446).epsilon(0.01));
  REQUIRE(constants::gravity == 9.80616);
  REQUIRE(constants::earth_radius == 6.37122e6);
  REQUIRE(constants::std_pressure == 101325);
  REQUIRE(constants::sidereal_day == 86164);
  REQUIRE(constants::calendar_day == 24*60*60);
  REQUIRE(constants::earth_omega * constants::sidereal_day == Approx(2*constants::pi));
  REQUIRE(constants::stefan_boltzmann == 5.67e-8);

  const double standard_pressure = 100000; // Pa
  const auto standard_temperature = constants::freezing_pt_h2o;
  const auto rdry = constants::rgas_dry_air;
  const auto rho = standard_pressure / (rdry * standard_temperature);
  const auto P = rho*rdry*constants::melting_pt_h2o;

  REQUIRE( P == Approx(standard_pressure));

  REQUIRE(constants::molec_weight_dry_air * 1000 == Approx(28.966));
  REQUIRE(constants::molec_weight_h2o * 1000 == Approx(18.016));
  REQUIRE(constants::molec_weight_h2so4 * 1000 == Approx(98.079));
  REQUIRE(constants::molec_weight_nh3 * 1000 == Approx(17.031).epsilon(0.01));
  REQUIRE(constants::molec_weight_so4 * 1000 == Approx(96.06));
  REQUIRE(constants::molec_weight_nh4 * 1000 == Approx(18.04).epsilon(0.01));
  REQUIRE(constants::molec_weight_co2 * 1000 == Approx(44.01));
  REQUIRE(constants::molec_weight_o3 * 1000 == Approx(48).epsilon(0.01));
  REQUIRE(constants::molec_weight_no2 * 1000 == Approx(44.013));
  REQUIRE(constants::molec_weight_co * 1000 == Approx(28.01));
  REQUIRE(constants::molec_weight_ch4 * 1000 == Approx(16.04).epsilon(0.001));
  REQUIRE(constants::molec_weight_o2 * 1000 == Approx(32).epsilon(0.001));
  REQUIRE(constants::molec_weight_n2 * 1000 == Approx(28).epsilon(0.001));

  REQUIRE(constants::triple_point_h2o == 273.16);
  REQUIRE(constants::density_h2o_liquid == 1000);
  REQUIRE(constants::density_h2o_ice == 917);
  REQUIRE(constants::cp_dry_air == 1.00464e3);
  REQUIRE(constants::cp_h2o_vapor == 1.81e3);
  REQUIRE(constants::cp_h2o_liq == 4.188e3);
  REQUIRE(constants::cp_h2o_ice == 2.11727e3);
  REQUIRE(constants::latent_heat_evap == 2.501e6);
  REQUIRE(constants::latent_heat_fusion == 3.337e5);

}
