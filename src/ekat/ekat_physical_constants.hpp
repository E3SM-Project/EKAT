#ifndef EKAT_PHYSICAL_CONSTANTS_HPP
#define EKAT_PHYSICAL_CONSTANTS_HPP

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




} // namespace ekat
#endif // EKAT_PHYSICAL_CONSTANTS
