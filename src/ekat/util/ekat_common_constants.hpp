#ifndef EKAT_COMMON_CONSTANTS_HPP
#define EKAT_COMMON_CONSTANTS_HPP

#include "ekat/ekat_physical_constants.hpp"

namespace ekat {
namespace common_constants {

const auto Pi = PhysicalConstant(3.14159265358979323846264, units::Units::nondimensional(), "Pi");

const auto Avogadro = PhysicalConstant(6.022214E23, units::pow(units::mol,-1), "Avogadro constant, N_A");

const auto Boltzmann = PhysicalConstant(1.38065E-23, units::J/units::K, "Boltzmann constant, k_B");

const auto R_gas_universal = Avogadro * Boltzmann;

const auto g_accel = PhysicalConstant(9.80616, units::m /units::pow(units::s,2),
  "gravity accerlation, g", "spherical geoid, constant radius");

const auto molec_weight_h20 = PhysicalConstant(18.016, units::g/units::mol,
  "molecular weight of water, M_w_h20");

const auto molec_weight_dry_air = PhysicalConstant(28.966, units::g/units::mol,
  "molecular weight of dry air, M_w_dry_air");

const auto rho_h20_liquid = PhysicalConstant(1.0E3,
  units::kg/units::pow(units::m,3), "rho_h20", "fresh water");

const auto Gamma_dry = PhysicalConstant(0.0098, units::K / units::m,
  "dry adiabatic lapse rate, Gamma_dry");


} // common_constants
} // namespace ekat
#endif // EKAT_COMMON_CONSTANTS_HPP
