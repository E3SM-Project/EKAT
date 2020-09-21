#ifndef EKAT_PHYSICAL_CONSTANTS

#define EKAT_PHYSICAL_CONSTANTS

#include "ekat/util/ekat_units.hpp"

namespace ekat {

class PhysicalConstant {
  public:
    using RealType = double;
    //enum Op {Times, Divide};

    // no default
    PhysicalConstant() = delete;

    PhysicalConstant(const PhysicalConstant&) = default;

    // Constructor for single value (i.e., NIST std)
    PhysicalConstant(const RealType& nvalue, const ekat::units::Units& units, const std::string& name) :
      m_e3sm_val(nvalue), m_nist_val(nvalue), m_units(units), m_name(name), m_info("") {}

    // Constructor for single value (i.e., NIST std)
    PhysicalConstant(const RealType& nvalue, const ekat::units::Units& units, const std::string& name,
      const std::string& info) :
      m_e3sm_val(nvalue), m_nist_val(nvalue), m_units(units), m_name(name), m_info(info) {}


    // Constructor for two different values (i.e., when E3SM value and NIST value are different)
    PhysicalConstant(const RealType& evalue, const RealType& nvalue, const ekat::units::Units& units,
      const std::string& name) : m_e3sm_val(evalue), m_nist_val(nvalue), m_units(units),
      m_name(name), m_info("") {}

    // Constructor for two different values (i.e., when E3SM value and NIST value are different)
    PhysicalConstant(const RealType& evalue, const RealType& nvalue, const ekat::units::Units& units,
      const std::string& name, const std::string& info) : m_e3sm_val(evalue), m_nist_val(nvalue), m_units(units),
      m_name(name), m_info(info) {}

    std::string name() const {return m_name;}

    std::string get_string() const {return m_name + " [" + to_string(m_units) +
      (m_info.empty() ? "]" : "] (" + m_info + ")");}

    std::string unit_string() const {return to_string(m_units);}

    units::Units get_units() const {return m_units;}

    RealType value() const {return m_e3sm_val;}

    RealType operator() () const {return value();}

  // allow subclasses for submodules (e.g., shoc, p3, haero, etc.)
  protected:
    RealType m_e3sm_val;
    RealType m_nist_val;
    units::Units m_units;

    std::string m_name;
    std::string m_info;

    friend bool operator == (const PhysicalConstant&, const PhysicalConstant&);
    friend PhysicalConstant operator*(const PhysicalConstant&, const PhysicalConstant&);
    friend PhysicalConstant operator/(const PhysicalConstant&, const PhysicalConstant&);
    friend std::string combine_info(const PhysicalConstant&, const PhysicalConstant&);
};

inline std::string combine_info(const PhysicalConstant& a, const PhysicalConstant& b) {
  std::string result("");
  if (a.m_info.empty()) {
    if (!b.m_info.empty()) {
      result = b.m_info;
    }
  }
  else {
    if (b.m_info.empty()){
      result = a.m_info;
    }
    else {
      result = a.m_info + "; " + b.m_info;
    }
  }
  return result;
}

inline bool operator == (const PhysicalConstant& lhs, const PhysicalConstant& rhs) {
  return lhs.m_e3sm_val == rhs.m_e3sm_val &&
         lhs.m_nist_val == rhs.m_nist_val &&
         lhs.m_units == rhs.m_units;
}

inline bool operator != (const PhysicalConstant& lhs, const PhysicalConstant& rhs) {
  return !(lhs == rhs);
}

inline PhysicalConstant operator*(const PhysicalConstant& lhs, const PhysicalConstant& rhs) {
  return PhysicalConstant(lhs.m_e3sm_val * rhs.m_e3sm_val,
                                    lhs.m_nist_val * rhs.m_nist_val,
                                    lhs.m_units * rhs.m_units,
                                    lhs.m_name + " * " + rhs.m_name,
                                    combine_info(lhs,rhs));
}

inline PhysicalConstant operator/(const PhysicalConstant& lhs, const PhysicalConstant& rhs) {
  return PhysicalConstant(lhs.m_e3sm_val / rhs.m_e3sm_val,
                                    lhs.m_nist_val / rhs.m_nist_val,
                                    lhs.m_units / rhs.m_units,
                                    lhs.m_name + " / " + rhs.m_name,
                                    combine_info(lhs, rhs));
}


// ================== COMMON PHYSICAL CONSTANTS (for examples only, at this point) =============== //
const auto Pi = PhysicalConstant(3.14159265358979323846264, units::Units::nondimensional(), "Pi");
const auto Avogadro = PhysicalConstant(6.022214E23, units::pow(units::mol,-1), "Avogadro constant, N_A");
const auto Boltzmann = PhysicalConstant(1.38065E-23, units::J/units::K, "Boltzmann constant, k_B");
const auto R_gas_universal = Avogadro * Boltzmann;
const auto g_accel = PhysicalConstant(9.80616, units::m /units::pow(units::s,2), "gravity accerlation, g", "spherical geoid, constant radius");
const auto molec_weight_h20 = PhysicalConstant(18.016, units::g/units::mol, "molecular weight of water, M_w_h20");
const auto molec_weight_dry_air = PhysicalConstant(28.966, units::g/units::mol, "molecular weight of dry air, M_w_dry_air");
const auto rho_h20_liquid = PhysicalConstant(1.0E3,
  units::kg/units::pow(units::m,3), "rho_h20", "fresh water");
const auto Gamma_dry = PhysicalConstant(0.0098, units::K / units::m, "dry adiabatic lapse rate, Gamma_dry");

} // namespace ekat
#endif // EKAT_PHYSICAL_CONSTANTS
