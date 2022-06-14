#include <catch2/catch.hpp>
#include "ekat/physics/base_constants.hpp"
#include <cmath>

using namespace ekat;
using namespace ekat::physics;

TEST_CASE("physics_constants", "[physics]") {

  using constants = BaseConstants<double>;

  REQUIRE(4*std::atan(1) == Approx(constants::pi));
  REQUIRE(constants::avogadro == Approx(6.02214076e23));
  REQUIRE(constants::boltzmann == Approx(1.380649e-23));
  REQUIRE(constants::r_gas == Approx(8.314462618));
  REQUIRE(constants::gravity == 9.80665);
  REQUIRE(constants::earth_radius == 6371e3);
  REQUIRE(constants::std_pressure == 101325);
  REQUIRE(constants::standard_pressure == 100000);
  REQUIRE(constants::sidereal_day == 86164.2);
  REQUIRE(constants::calendar_day == 24*60*60);
  REQUIRE(constants::earth_omega * constants::sidereal_day == Approx(2*constants::pi));
  REQUIRE(constants::stefan_boltzmann == Approx(5.670374419e-8));
  REQUIRE(constants::karman == 0.4);

  const auto standard_temperature = constants::freezing_pt_h2o;
  const auto rdry = constants::rgas_dry_air;
  const auto rho = constants::standard_pressure / (rdry * standard_temperature);
  const auto P = rho*rdry*constants::freezing_pt_h2o;

  REQUIRE( P == Approx(constants::standard_pressure));

  REQUIRE(constants::molec_weight_c == Approx(0.0120107));
  REQUIRE(constants::molec_weight_ch4 == Approx( 0.0160425 ));;
  REQUIRE(constants::molec_weight_co == Approx( 0.0280101 ));;
  REQUIRE(constants::molec_weight_co2 == Approx( 0.0440095 ));;
  REQUIRE(constants::molec_weight_dry_air == Approx( 0.028963 ));;
  REQUIRE(constants::molec_weight_h2o == Approx( 0.0180153 ));;
  REQUIRE(constants::molec_weight_h2so4 == Approx( 0.098078 ));;
  REQUIRE(constants::molec_weight_n2 == Approx( 0.0280134 ));;
  REQUIRE(constants::molec_weight_nh3 == Approx( 0.0170305));;
  REQUIRE(constants::molec_weight_nh4 == Approx( 0.0180390));;
  REQUIRE(constants::molec_weight_no2 == Approx( 0.0460055 ));;
  REQUIRE(constants::molec_weight_o == Approx( 0.0159994 ));;
  REQUIRE(constants::molec_weight_o2 == Approx( 0.0319988 ));;
  REQUIRE(constants::molec_weight_o3 == Approx( 0.0479982 ));;
  REQUIRE(constants::molec_weight_so4 == Approx( 0.096064 ));;
  REQUIRE(constants::carbon_13_to_12_ratio == 0.011179);
  REQUIRE(constants::density_h2o_liq_0c == Approx( 999.8428 ));
  REQUIRE(constants::density_h2o_ice_0c == Approx( 916.7 ));
  REQUIRE(constants::freezing_pt_h2o == Approx( 273.15 ));
  REQUIRE(constants::critical_temp_h2o == Approx( 647.096 ));
  REQUIRE(constants::latent_heat_evap_0c == Approx( 2501000) );
  REQUIRE(constants::latent_heat_fusion == Approx(   333700) );
  REQUIRE(constants::latent_heat_sublimation == Approx( 2834700) );
  REQUIRE(constants::rgas_h2o_vapor == Approx(461.522));
  REQUIRE(constants::surface_tension_h2o_air_0c == Approx( 0.07570689080553497 ));
  REQUIRE(constants::cp_h2o_liq_0c == Approx( 4219.4 ));
  REQUIRE(constants::cp_h2o_vapor == Approx( 1846) );
  REQUIRE(constants::cp_h2o_ice == Approx( 2110) );
  REQUIRE(constants::cp_dry_air == Approx( 1004.64 ));
  REQUIRE(constants::triple_point_h2o == Approx( 273.16 ));
  REQUIRE(constants::cv_dry_air == Approx( 717.6 ));
  REQUIRE(constants::thermal_conductivity_h2o_ice_0c == 2.1);
  REQUIRE(constants::rgas_dry_air == Approx(287.072));
  REQUIRE(constants::cp_dry_air == 1004.64);
  REQUIRE(constants::vsmow_ratio_18o_16o == Approx(0.200520));
  REQUIRE(constants::vsmow_ratio_17o_16o == Approx(0.03769));
  REQUIRE(constants::vsmow_ratio_2h_h == 0.015576);


/*
  Below this line are constants defined in shr_const_mod.F90, but not in EKAT

    They are either derived constants, written in terms of the base constants
    defined above, or they are state-dependent and not actually constant across
    the entire coupled model.

*/
//   REQUIRE(constants::weight_ratio_h2o_air == Approx(0.62197).epsilon(0.00001));
//   REQUIRE(constants::vsmow_ratio_o16_total_o == 0.997628);
//   REQUIRE(constants::melting_pt_h2o == Approx( 273.15 ));
//   REQUIRE(constants::density_seawater == 1026);
//   REQUIRE(constants::freezing_pt_seawater == Approx(271.35));
//   REQUIRE(constants::ocean_surface_layer_depth == 3);
//   REQUIRE(constants::ocean_ref_salinity == 34.7);
//   REQUIRE(constants::cp_seawater == 3996);
//   REQUIRE(constants::vsmow_ratio_3h_h == 1.85e-6);
//   REQUIRE(constants::vsmow_ratio_h_total_h == 0.99984426);

//   REQUIRE(constants::ice_ref_salinity == 4);
//   REQUIRE(constants::freezing_pt_h2o_zerop == 273.2122);
//   REQUIRE(constants::dfreeze_zerop_dp == -7.43e-8);
//   REQUIRE(constants::dfreeze_zerop_ds == -5.63e-2);
//   REQUIRE(constants::dfreeze_zerop_dpds == Approx(-1.74e-10));

//   REQUIRE(constants::gas_const_ratio_zvir ==
//     constants::rgas_h2o_vapor/constants::rgas_dry_air - 1);
//   REQUIRE(constants::specific_heat_ratio_cpvir ==
//     constants::cp_h2o_vapor/constants::cp_dry_air -1);
}
