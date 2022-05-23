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
   auto-generated Fortran constants module.

 */

template <typename Scalar>
struct Constants
{
  /**

      General and Earth-related constants

  */
  /// EKAT_CONSTANTS_START_PARSE
  /// pi
  static constexpr Scalar pi = 3.14159265358979323846264;
  /// 1/6th of the pi
  static constexpr Scalar pi_sixth = pi/6;
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
  /// standard pressure [Pa]
  static constexpr Scalar std_pressure = 101325.0;
  /// sidereal day [s]
  static constexpr Scalar sidereal_day = 86164;
  /// calendar day [s]
  static constexpr Scalar calendar_day = 86400;
  /// earth's angular rotation rate
  static constexpr Scalar earth_omega = 2*pi/sidereal_day;
  /// stefan-boltzmann constant [W/(m2 K4)]
  static constexpr Scalar stefan_boltzmann = 5.67e-8;

  /**

    Chemistry constants

  */
  /// Molecular weight of water [kg/mol]
  static constexpr Scalar molec_weight_h2o = 0.018016;
  /// Molecular weight of dry air [kg/mol]
  static constexpr Scalar molec_weight_dry_air = 0.028966;
  /// Molecular weight of sulfuric acid @f$\text{H}_2\text{SO}_4@f$ [kg/mol]
  static constexpr Scalar molec_weight_h2so4 = 0.098079;
  /// Molecular weight of ammonia @f$\text{N}\text{H}_3@f$ [kg/mol]
  static constexpr Scalar molec_weight_nh3 = 0.01703052;
  /// Molecular weight of sulfate ion @f$\text{SO}_4^{2-}@f$ [kg/mol]
  static constexpr Scalar molec_weight_so4 = 0.09606;
  /// Molecular weight of ammonium ion @f$\text{NH}_4^+@f$ [kg/mol]
  static constexpr Scalar molec_weight_nh4 = 0.018039;
  /// Molecular weight of carbon dioxide @f$\text{CO}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_co2 = 0.0440095;
  /// Molecular weight of ozone @f$\text{O}_3@f$ [kg/mol]
  static constexpr Scalar molec_weight_o3 = 0.0479982;
  /// Molecular weight of nitrous oxide @f$\text{NO}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_no2 = 0.0440128;
  /// Molecular weight of carbon monoxide @f$\text{CO}@f$ [kg/mol]
  static constexpr Scalar molec_weight_co = 0.0280101;
  /// Molecular weight of methane @f$\text{CH}_4@f$ [kg/mol]
  static constexpr Scalar molec_weight_ch4 = 0.01604246;
  /// Molecular weight of oxygen @f$\text{O}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_o2 = 0.031998;
  /// Molecular weight of nitrogen @f$\text{N}_2@f$ [kg/mol]
  static constexpr Scalar molec_weight_n2 = 0.0280134;

  /**

    water constants

  */
  /// Ratio of molecular weights of water and dry air (often denoted \epsilon)
  static constexpr Scalar weight_ratio_h20_air = molec_weight_h2o / molec_weight_dry_air;
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

    dry air

  */
  /// gas constant for dry air
  static constexpr Scalar rgas_dry_air = r_gas / molec_weight_dry_air;
  /// specific heat at constant pressure for dry air
  static constexpr Scalar cp_dry_air = 1004.64;

  /// EKAT_CONSTANTS_END_PARSE
};

} // namespace physics
} // namespace scream

#endif
