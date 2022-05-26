#ifndef PHYSICS_CONSTANTS_HPP
#define PHYSICS_CONSTANTS_HPP

#include "ekat/util/ekat_string_utils.hpp"
#include "ekat/ekat_scalar_traits.hpp"

#include <vector>

namespace ekat {
namespace physics {

/*
   ALL CONSTANTS ARE DEFINED IN SI UNITS
   client modules must convert to other units as necessary

   time:                seconds   [s]
   length:              meters    [m]
   mass:                kilograms [kg]
   temperature:         Kelvin    [K]
   amount of substance: mole      [mol]
   pressure:            Pascals   [Pa]
   force:               Newtons   [N]
   work:                Joules    [J]

   Each constant below must be preceded by a comment line beginning with ///
   to enable the CMake parser to use the same numeric values for the
   auto-generated .cpp file and Fortran constants module.

  All constants must be placed between the lines containing the uppercase strings
  "ekat_constants_start_parse" and "ekat_constants_end_parse".
 */

template <typename Scalar>
struct Constants
{
  static_assert(std::is_floating_point<Scalar>::value, "floating point type required.");
  /**

      General and Earth-related constants

  */
  /// EKAT_CONSTANTS_START_PARSE
  /// pi
  static constexpr Scalar pi = 3.14159265358979323846264;
  /// Avogadro's constant [#/mol]
  static constexpr Scalar avogadro = 6.022214076e23;
  /// Boltzmann's constant [J/K]
  static constexpr Scalar boltzmann = 1.380649e-23;
  /// Universal gas constant [J/(K mol)]
  static constexpr Scalar r_gas = avogadro*boltzmann;
  /// acceleration of gravity [m/s^2]
  static constexpr Scalar gravity = 9.80616;
  /// radius of a spherical Earth [m]
  static constexpr Scalar earth_radius = 6.37122e6;
  /// standard pressure [Pa] (1 atm, pre-1982 definition)
  static constexpr Scalar std_pressure = 101325.0;
  /// standard pressure [Pa] (1 bar, post-1982 definition)
  static constexpr Scalar standard_pressure = 100000.0;
  /// sidereal day [s]
  static constexpr Scalar sidereal_day = 86164;
  /// calendar day [s]
  static constexpr Scalar calendar_day = 86400;
  /// earth's angular rotation rate
  static constexpr Scalar earth_omega = 2*pi/sidereal_day;
  /// stefan-boltzmann constant [W/(m2 K4)]
  static constexpr Scalar stefan_boltzmann = 5.67e-8;
  /// Karman constant
  static constexpr Scalar karman = 0.4;

  /**

    Chemistry constants

  */
  /// Molecular weight of carbon [kg/mol]
  static constexpr Scalar molec_weight_c = 0.0120107;
  /// Molecular weight of methane @f$\text{CH}_4@f$ [kg/mol]
  static constexpr Scalar molec_weight_ch4 = 0.01604246;
  /// Molecular weight of carbon monoxide @f$\text{CO}@f$ [kg/mol]
  static constexpr Scalar molec_weight_co = 0.0280101;
  /// Molecular weight of carbon dioxide @f$\text{CO}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_co2 = 0.0440095;
  /// Molecular weight of dry air [kg/mol]
  static constexpr Scalar molec_weight_dry_air = 0.028966;
  /// Molecular weight of water [kg/mol]
  static constexpr Scalar molec_weight_h2o = 0.018016;
  /// Molecular weight of sulfuric acid @f$\text{H}_2\text{SO}_4@f$ [kg/mol]
  static constexpr Scalar molec_weight_h2so4 = 0.098079;
  /// Molecular weight of nitrogen @f$\text{N}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_n2 = 0.0280134;
  /// Molecular weight of ammonia @f$\text{N}\text{H}_3@f$ [kg/mol]
  static constexpr Scalar molec_weight_nh3 = 0.01703052;
  /// Molecular weight of ammonium ion @f$\text{NH}_4^+@f$ [kg/mol]
  static constexpr Scalar molec_weight_nh4 = 0.018039;
  /// Molecular weight of nitrous oxide @f$\text{NO}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_no2 = 0.0440128;
  /// Molecular weight of oxygen atom [kg/mol]
  static constexpr Scalar molec_weight_o = 0.0159994;
  /// Molecular weight of oxygen @f$\text{O}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_o2 = 0.031998;
  /// Molecular weight of ozone @f$\text{O}_3@f$ [kg/mol]
  static constexpr Scalar molec_weight_o3 = 0.0479982;
  /// Molecular weight of sulfate ion @f$\text{SO}_4^{2-}@f$ [kg/mol]
  static constexpr Scalar molec_weight_so4 = 0.09606;
  /// ratio of carbon isotopes in Pee Dee Belemnite standard
  static constexpr Scalar carbon_13_to_12_ratio = 0.0112372;

  /**

    water constants

  */
  /// Ratio of molecular weights of water and dry air (often denoted \epsilon)
  static constexpr Scalar weight_ratio_h2o_air = molec_weight_h2o / molec_weight_dry_air;
  /// density of freshwater
  static constexpr Scalar density_h2o_liquid = 1e3;
  /// density of freshwater ice
  static constexpr Scalar density_h2o_ice = 917;
  /// freeze/melt point
  static constexpr Scalar freezing_pt_h2o = 273.15;
  /// freeze/melt point
  static constexpr Scalar melting_pt_h2o = freezing_pt_h2o;
  /// latent heat
  static constexpr Scalar latent_heat_evap = 2.501e6;
  /// latent heat
  static constexpr Scalar latent_heat_fusion = 3.337e5;
  /// latent heat
  static constexpr Scalar latent_heat_sublimation = latent_heat_fusion + latent_heat_evap;
  /// gas constant for water vapor
  static constexpr Scalar rgas_h2o_vapor = r_gas / molec_weight_h2o;
  /// flat surface tension at freeze point
  static constexpr Scalar surface_tension_h2o_air_273k = 0.07564;
  /// specific heat of water at constant pressure [J/kg/K]
  static constexpr Scalar cp_h2o_liq = 4188.0;
  /// specific heat of water vapor at constant pressure
  static constexpr Scalar cp_h2o_vapor = 1810;
  /// specific heat of water ice at constant pressure
  static constexpr Scalar cp_h2o_ice = 2117.27;
  /// triple point temperature [K]
  static constexpr Scalar triple_point_h2o = 273.16;
  /**
    seawater / ocean
  */
  /// density of seawater
  static constexpr Scalar density_seawater = 1026;
  /// freeze/melt point
  static constexpr Scalar freezing_pt_seawater = freezing_pt_h2o - 1.8;
  /// ocean surface layer depth [m] for diurnal SST
  static constexpr Scalar ocean_surface_layer_depth = 3;
  /// ocean reference salinity (psu)
  static constexpr Scalar ocean_ref_salinity = 34.7;
  /// specific heat of seawater
  static constexpr Scalar cp_seawater = 3996;
  /**
    vmsow : Vienna mean standard ocean water
  */
  /// ratio of O18 to O16
  static constexpr Scalar vmsow_ratio_o18_o16 = 2005.2e-6;
  /// ratio of O17 to O16
  static constexpr Scalar vmsow_ratio_o17_o16 = 379e-6;
  /// ratio of O16 to total oxygen
  static constexpr Scalar vmsow_ratio_o16_total_o = 0.997628;
  /// ratio of deuterium to hydrogen
  static constexpr Scalar vmsow_ratio_2h_h = 155.76e-6;
  /// ratio of tritium to hydrogen
  static constexpr Scalar vmsow_ratio_3h_h = 1.85e-6;
  /// ratio of hydogren to total hydrogen
  static constexpr Scalar vmsow_ratio_h_total_h = 0.99984426;


  /**
    ice
  */
  /// ice reference salinity [psu]
  static constexpr Scalar ice_ref_salinity = 4;
  /// freezing point at zero pressure in an sub-shelf ice cavity
  static constexpr Scalar freezing_pt_h2o_zerop = 273.2122;
  /// linear coefficient sub-shelf freezing pt change vs pressure [K/Pa]
  static constexpr Scalar dfreeze_zerop_dp = -7.43e-8;
  /// linear coefficient sub-shelf freezing pt change vs salinity [K/psu]
  static constexpr Scalar dfreeze_zerop_ds = -5.63e-2;
  /// linear coefficient sub-shelf freezing pt change vs salinity and pressure [K/ (Pa psu)]
  static constexpr Scalar dfreeze_zerop_dpds = -1.74e-10;
  /// thermal conductivity of land ice [W/m/K]
  static constexpr Scalar thermal_conductivity_ice = 2.1;
  /// thermal diffusivity of land ice (denoted kappa)
  static constexpr Scalar thermal_diffusivity_landice = thermal_conductivity_ice/(density_h2o_ice*cp_h2o_ice);

  /**

    dry air

  */
  /// gas constant for dry air
  static constexpr Scalar rgas_dry_air = r_gas / molec_weight_dry_air;
  /// specific heat at constant pressure for dry air
  static constexpr Scalar cp_dry_air = 1004.64;
  /// Ratio of water vapor and dry air gas constants, minus one (denoted zvir in e3sm)
  static constexpr Scalar gas_const_ratio_zvir = rgas_h2o_vapor/rgas_dry_air - 1;
  /// Ratio of water vapor and dry air specific heats, minus one (denoted cpvir in e3sm)
  static constexpr Scalar specific_heat_ratio_cpvir = cp_h2o_vapor/cp_dry_air - 1;

  /// EKAT_CONSTANTS_END_PARSE
};

} // namespace physics
} // namespace scream

#endif
