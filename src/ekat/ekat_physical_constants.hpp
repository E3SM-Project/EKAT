#ifndef EKAT_PHYSICAL_CONSTANTS
#define EKAT_PHYSICAL_CONSTANTS

#include "ekat/util/ekat_units.hpp"

namespace ekat {

template <typename RealType=double>
class PhysicalConstant {
  public:
    //enum Op {Times, Divide};

    // no default
    PhysicalConstant() = delete;

    PhysicalConstant(const PhysicalConstant<RealType>&) = default;

    // Constructor for single value (i.e., NIST std)
    PhysicalConstant(const RealType& nvalue, const ekat::units::Units& units, const std::string& name) :
      m_nist_val(nvalue), m_e3sm_val(nvalue), m_units(units), m_name(name), m_info("") {}

    // Constructor for single value (i.e., NIST std)
    PhysicalConstant(const RealType& nvalue, const ekat::units::Units& units, const std::string& name,
      const std::string& info) :
      m_nist_val(nvalue), m_e3sm_val(nvalue), m_units(units), m_name(name), m_info(info) {}


    // Constructor for two different values (i.e., when E3SM value and NIST value are different)
    PhysicalConstant(const RealType& evalue, const RealType& nvalue, const ekat::units::Units& units,
      const std::string& name) : m_e3sm_val(evalue), m_nist_val(nvalue), m_units(units),
      m_name(name), m_info("") {}

    // Constructor for two different values (i.e., when E3SM value and NIST value are different)
    PhysicalConstant(const RealType& evalue, const RealType& nvalue, const ekat::units::Units& units,
      const std::string& name, const std::string& info) : m_e3sm_val(evalue), m_nist_val(nvalue), m_units(units),
      m_name(name), m_info(info) {}

    // Constructor from the product of two other PhysicalConstants
    PhysicalConstant(const PhysicalConstant& a, const PhysicalConstant& b, const std::string& name,
      const std::string& info="") : m_nist_val(a.m_nist_val*b.m_nist_val),
        m_e3sm_val(a.m_e3sm_val*b.m_e3sm_val), m_units(a.m_units*b.m_units),
        m_name(name), m_info(info) {}

    std::string name() const {return m_name;}

    std::string get_string() const {return m_name + " [" + to_string(m_units) +
      (m_info.empty() ? "]" : "] (" + m_info + ")");}

    std::string unit_string() const {return to_string(m_units);}

    RealType value() const {return m_e3sm_val;}

    RealType operator() () const {return value();}

  // allow subclasses for submodules (e.g., shoc, p3, haero, etc.)
  protected:
    RealType m_nist_val;
    RealType m_e3sm_val;
    ekat::units::Units m_units;

    std::string m_name;
    std::string m_info;

    friend bool operator == (const PhysicalConstant&, const PhysicalConstant&);
};

template <typename RealType>
inline bool operator == (const PhysicalConstant<RealType>& lhs, const PhysicalConstant<RealType>& rhs) {
  return lhs.m_e3sm_val == rhs.m_e3sm_val &&
         lhs.m_nist_val == rhs.m_nist_val &&
         lhs.m_units == rhs.m_units;
}

template <typename RealType>
inline bool operator != (const PhysicalConstant<RealType>& lhs, const PhysicalConstant<RealType>& rhs) {
  return !(lhs == rhs);
}

// ================== COMMON PHYSICAL CONSTANTS (for examples only, at this point) =============== //
const auto Pi = PhysicalConstant<>(3.14159265358979323846264, ekat::units::Units::nondimensional(), "Pi");
const auto Avogadro = PhysicalConstant<>(6.022214E23, ekat::units::pow(ekat::units::mol,-1), "Avogadro constant, N_A");
const auto Boltzmann = PhysicalConstant<>(1.38065E-23, ekat::units::J/ekat::units::K, "Boltzmann constant, k_B");
const auto R_gas_universal = PhysicalConstant<>(Avogadro, Boltzmann, "Universal gas constant, R");
const auto g_accel = PhysicalConstant<>(9.80616, ekat::units::m /ekat::units::pow(ekat::units::s,2), "gravity accerlation, g", "spherical geoid, constant radius");
const auto molec_weight_h20 = PhysicalConstant<>(18.016, ekat::units::g/ekat::units::mol, "molecular weight of water, M_w_h20");
const auto molec_weight_dry_air = PhysicalConstant<>(28.966, ekat::units::g/ekat::units::mol, "molecular weight of dry air, M_w_dry_air");
const auto rho_h20_liquid = PhysicalConstant<>(1.0E3,
  ekat::units::kg/ekat::units::pow(ekat::units::m,3), "rho_h20", "fresh water");
const auto Gamma_dry = PhysicalConstant<>(0.0098, ekat::units::K / ekat::units::m, "dry adiabatic lapse rate, Gamma_dry");

} // namespace ekat
#endif // EKAT_PHYSICAL_CONSTANTS
