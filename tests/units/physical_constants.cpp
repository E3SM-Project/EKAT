#include <catch2/catch.hpp>

#include "ekat/ekat_physical_constants.hpp"

#include <iostream>

TEST_CASE("physical_constants", "") {
  using namespace ekat;
  using namespace ekat::units;

  SECTION("Universal constants") {
    std::cout << Pi() << " " << Pi.get_string() << '\n';
    std::cout << Avogadro() << " " <<Avogadro.get_string() << '\n';
    std::cout << Boltzmann() << " " << Boltzmann.get_string() << '\n';
    std::cout << R_gas_universal() <<  " " << R_gas_universal.get_string() << '\n';
  }

  SECTION("Shallow atmosphere constants") {
    std::cout << g_accel() << " " << g_accel.get_string() << '\n';
    std::cout << molec_weight_dry_air() << " " << molec_weight_dry_air.get_string() << '\n';
    std::cout << Gamma_dry() << " " << Gamma_dry.get_string() << '\n';
  }

  SECTION("Water constants") {
   std::cout << molec_weight_h20() << " " << molec_weight_h20.get_string() << '\n';
   std::cout << rho_h20_liquid() << " " << rho_h20_liquid.get_string() << '\n';
  }
}
