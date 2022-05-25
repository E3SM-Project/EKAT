#include <catch2/catch.hpp>
#include "ekat/physics/constants.hpp"
#include <cmath>

using namespace ekat;
using namespace ekat::physics;

TEST_CASE("physics_constants", "[physics]") {

  using constants = Constants<double>;

  REQUIRE(4*std::atan(1) == Approx(constants::pi));
  REQUIRE(constants::avogadro == Approx(6.022e23).epsilon(0.01));
  REQUIRE(constants::boltzmann == Approx(1.38e-23).epsilon(0.01));
  REQUIRE(constants::r_gas == Approx(8.31446).epsilon(0.01));
  REQUIRE(constants::gravity == 9.80616);
  REQUIRE(constants::earth_radius == 6.37122e6);
  REQUIRE(constants::std_pressure == 101325);
  REQUIRE(constants::standard_pressure == 100000);
  REQUIRE(constants::sidereal_day == 86164);
  REQUIRE(constants::calendar_day == 24*60*60);
  REQUIRE(constants::earth_omega * constants::sidereal_day == Approx(2*constants::pi));
  REQUIRE(constants::stefan_boltzmann == 5.67e-8);
  REQUIRE(constants::karman == 0.4);

  const auto standard_temperature = constants::freezing_pt_h2o;
  const auto rdry = constants::rgas_dry_air;
  const auto rho = constants::standard_pressure / (rdry * standard_temperature);
  const auto P = rho*rdry*constants::melting_pt_h2o;

  REQUIRE( P == Approx(constants::standard_pressure));

  REQUIRE(constants::molec_weight_c * 1000 == Approx(12.0107));
  REQUIRE(constants::molec_weight_ch4 * 1000 == Approx(16.04).epsilon(0.001));
  REQUIRE(constants::molec_weight_co * 1000 == Approx(28.01));
  REQUIRE(constants::molec_weight_co2 * 1000 == Approx(44.01));
  REQUIRE(constants::molec_weight_dry_air * 1000 == Approx(28.966));
  REQUIRE(constants::molec_weight_h2o * 1000 == Approx(18.016));
  REQUIRE(constants::molec_weight_h2so4 * 1000 == Approx(98.079));
  REQUIRE(constants::molec_weight_n2 * 1000 == Approx(28.0134));
  REQUIRE(constants::molec_weight_nh3 * 1000 == Approx(17.03052));
  REQUIRE(constants::molec_weight_nh4 * 1000 == Approx(18.04).epsilon(0.01));
  REQUIRE(constants::molec_weight_no2 * 1000 == Approx(44.013));
  REQUIRE(constants::molec_weight_o * 1000 == Approx(15.9994));
  REQUIRE(constants::molec_weight_o2 * 1000 == Approx(31.998));
  REQUIRE(constants::molec_weight_o3 * 1000 == Approx(47.9982));
  REQUIRE(constants::molec_weight_so4 * 1000 == Approx(96.06));
  REQUIRE(constants::carbon_13_to_12_ratio == 0.0112372);

  REQUIRE(constants::weight_ratio_h2o_air == Approx(0.62197).epsilon(0.00001));
  REQUIRE(constants::density_h2o_liquid == 1000);
  REQUIRE(constants::density_h2o_ice == 917);
  REQUIRE(constants::freezing_pt_h2o == 273.15);
  REQUIRE(constants::melting_pt_h2o == 273.15);
  REQUIRE(constants::latent_heat_evap == 2.501e6);
  REQUIRE(constants::latent_heat_fusion == 3.337e5);
  REQUIRE(constants::latent_heat_sublimation == 2834700);
  REQUIRE(constants::rgas_h2o_vapor == Approx(461.501).epsilon(0.001));
  REQUIRE(constants::surface_tension_h2o_air_273k == 0.07564);
  REQUIRE(constants::cp_h2o_liq == 4.188e3);
  REQUIRE(constants::cp_h2o_vapor == 1.81e3);
  REQUIRE(constants::cp_h2o_ice == 2.11727e3);
  REQUIRE(constants::cp_dry_air == 1.00464e3);
  REQUIRE(constants::triple_point_h2o == 273.16);

  REQUIRE(constants::density_seawater == 1026);
  REQUIRE(constants::freezing_pt_seawater == Approx(271.35));
  REQUIRE(constants::ocean_surface_layer_depth == 3);
  REQUIRE(constants::ocean_ref_salinity == 34.7);
  REQUIRE(constants::cp_seawater == 3996);

  REQUIRE(constants::vmsow_ratio_o18_o16 == 2005.2e-6);
  REQUIRE(constants::vmsow_ratio_o17_o16 == 379e-6);
  REQUIRE(constants::vmsow_ratio_o16_total_o == 0.997628);
  REQUIRE(constants::vmsow_ratio_2h_h == 155.76e-6);
  REQUIRE(constants::vmsow_ratio_3h_h == 1.85e-6);
  REQUIRE(constants::vmsow_ratio_h_total_h == 0.99984426);

  REQUIRE(constants::ice_ref_salinity == 4);
  REQUIRE(constants::freezing_pt_h2o_zerop == 273.2122);
  REQUIRE(constants::dfreeze_zerop_dp == -7.43e-8);
  REQUIRE(constants::dfreeze_zerop_ds == -5.63e-2);
  REQUIRE(constants::thermal_conductivity_ice == 2.1);

  REQUIRE(constants::rgas_dry_air == Approx(287.0420493).epsilon(0.001));
  REQUIRE(constants::cp_dry_air == 1004.64);
  REQUIRE(constants::gas_const_ratio_zvir ==
    constants::rgas_h2o_vapor/constants::rgas_dry_air - 1);
  REQUIRE(constants::specific_heat_ratio_cpvir ==
    constants::cp_h2o_vapor/constants::cp_dry_air -1);
}
