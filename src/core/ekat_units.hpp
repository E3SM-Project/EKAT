#ifndef EKAT_UNITS_HPP
#define EKAT_UNITS_HPP

#include "ekat_rational_constant.hpp"
#include "ekat_scaling_factor.hpp"
#include "ekat_string_utils.hpp"

#include <array>
#include <limits>
#include <string_view>

namespace ekat
{

namespace units
{

constexpr int UNITS_MAX_STR_LEN = 128;
constexpr int NUM_BASIC_UNITS = 7;

/*
 *  Units: a class to store physical units in terms of fundamental ones
 *
 *  Units is basically storing 8 numbers:
 *   - a scaling factor
 *   - the exponents of the 7 base units
 *  So if a quantity has units kPa, it will store
 *
 *    - a scaling factor of 1000
 *    - the exponents [ -1, -2, 1, 0, 0, 0, 0 ]
 *
 *  since kPa = 1000 kg m^-1 s ^-2.
 *
 *  A few arithmetic operators as well as pow/sqrt functions are overloaded,
 *  to allow easy construction of derived units from fundamental/common ones.
 *
 *  Note: this class can be used in constant expressions.
 *
 *  Note: we do *not* overload operator^. Although it would be nice to write
 *            auto my_units = m^2 / s^2;
 *        it would be very bug prone, since ^ has lower precedence than * /.
 *        Sure, using parentheses makes it safe, but then you may as well write
 *            auto my_units = pow(m,2) / pow(s,2);
 *            auto my_units = (m*m) / (s*s);
 *
 *  Note: there should be no need to store a string, since we can build a string
 *        on the fly when printing the units. However, some apps might prefer to
 *        use common strings for certain units. E.g., for 'auto N = kg*m/(s*s)'
 *        it might be preferable to print "N" rather than "kg*m/(s*s)".
 *        If you want a derived unit to print a name different from what you get
 *        combining scaling factor and the fundamental units exponents,
 *        you *must* pass the name at construction time.
 */

class Units {
public:

  // No default
  Units () = delete;

  // Construct a non-dimensional quantity
  constexpr Units (const ScalingFactor& scaling)
   : Units(0,0,0,0,0,0,0,scaling,scaling.string_repr<UNITS_MAX_STR_LEN>().data())
  {
    // Nothing to do here
  }

  // Construct a general quantity
  constexpr Units (const RationalConstant& lengthExp,
                   const RationalConstant& timeExp,
                   const RationalConstant& massExp,
                   const RationalConstant& temperatureExp,
                   const RationalConstant& currentExp,
                   const RationalConstant& amountExp,
                   const RationalConstant& luminousIntensityExp,
                   const ScalingFactor& scalingFactor = ScalingFactor::one(),
                   const std::string_view& n = "")
   : m_scaling {scalingFactor}
   , m_units {lengthExp,timeExp,massExp,temperatureExp,currentExp,amountExp,luminousIntensityExp}
  {
    assert(n.size() < UNITS_MAX_STR_LEN);

    for (size_t i = 0; i < n.size(); ++i)
        m_string_repr[i] = n[i];

    m_string_repr[n.size()] = '\0';
  }

  constexpr Units (const Units& rhs, const std::string_view& s)
   : Units(rhs)
  {
    assert (s.size()<UNITS_MAX_STR_LEN);
    for (size_t i=0; i<s.size(); ++i) {
      m_string_repr[i] = s[i];
    }
    m_string_repr[s.size()] = '\0';
  }
  constexpr Units (const Units&) = default;

  constexpr Units& operator= (const Units&) = default;

  [[deprecated("Use the 'ekat::units::none' constant instead.")]]
  static constexpr Units nondimensional () {
    Units u(ScalingFactor::one());
    u.m_string_repr[0] = '1';
    return u;
  }
  static constexpr Units invalid () {
    constexpr auto infty = std::numeric_limits<RationalConstant::iType>::max();
    return Units(0,0,0,0,0,0,0,ScalingFactor(infty));
  }

  std::string get_si_string () const {
    static constexpr std::string_view symbols[] = {"m", "s", "kg", "K", "A", "mol", "cd"};
    std::string s;
    for (int i=0; i<NUM_BASIC_UNITS; ++i) {
      if (m_units[i].num==0) {
        continue;
      }
      s += symbols[i];
      if (m_units[i]!=RationalConstant::one()) {
        s += "^" + m_units[i].to_string(Format::Rat);
      }
      s += " ";
    }

    // Prepend the scaling only if it's not one, or if this is a dimensionless unit
    if (m_scaling!=ScalingFactor::one() || is_dimensionless()) {
      s = m_scaling.to_string(Format::Rat) + " " + s;
    }

    // Remove leading/trailing whitespaces
    return trim(s);
  }

  std::string to_string () const {
    // Remove trailing/leading spaces from string repr
    return trim(std::string(m_string_repr.data()));
  }

  constexpr bool is_dimensionless () const {
    return m_units[0].num==0 &&
           m_units[1].num==0 &&
           m_units[2].num==0 &&
           m_units[3].num==0 &&
           m_units[4].num==0 &&
           m_units[5].num==0 &&
           m_units[6].num==0;
  }

  constexpr bool is_scaled () const {
    return m_scaling!=ScalingFactor::one();
  }

  // Returns true if this Units has its own symbol.
  // E.g., for "J" it returns true, but for "J/s" it returns false
  constexpr bool has_symbol () const {
    for (const auto& c : m_string_repr) {
       if (c=='*' or c=='/' or c=='^' or c==' ' or c=='(' or c==')')
         return false;
       if (c=='\0')
         return true;
    }
    return true;
  }

  /**
   * Returns a copy of the unit with a new string representation.
   * Use [[nodiscard]] to ensure users get a warning if the returned
   * value is not used.
   */
  [[nodiscard]] constexpr Units rename(const std::string_view s) const {
    return Units(*this, s);
  }
private:
  constexpr std::string_view string_repr () const {
    return m_string_repr.data();
  }

  // Returns true if the unit is composite (i.e., does not have a one-word symbol,
  // but instead is a compound of symbols, such as a*b/c)
  static constexpr bool composite (const std::string_view sv) {
    for (const auto& c : sv) {
      // These are the characters that trigger parentheses
      if (c == '*' || c == '/' || c == '^')
        return true;
    }
    return false;
  }

  static constexpr std::array<char, UNITS_MAX_STR_LEN>
  concat_repr (const std::string_view& lhs,
               const std::string_view& rhs,
               const char sep)
  {
    // 1. Calculate required length upfront
    const bool comp1 = composite(lhs);
    const bool comp2 = composite(rhs);
    
    const size_t total_len = lhs.size() + rhs.size() + 
                             (comp1 ? 2 : 0) + (comp2 ? 2 : 0) + 
                             (sep != '\0' ? 1 : 0);
    // Trigger a compiler error if the name is too long.
    assert (total_len<UNITS_MAX_STR_LEN);

    std::array<char, UNITS_MAX_STR_LEN> out = {'\0'};

    int pos = 0;
    // Copy LHS
    if (comp1) out[pos++] = '(';
    for (char c : lhs) out[pos++] = c;
    if (comp1) out[pos++] = ')';

    // Add separator (if needed)
    if (sep != '\0') { out[pos++] = sep; }

    // Copy RHS
    if (comp2) out[pos++] = '(';
    for (char c : rhs) out[pos++] = c;
    if (comp2) out[pos++] = ')';

    return out;
  }
private:

  friend constexpr bool operator==(const Units&,const Units&);

  friend constexpr Units operator*(const Units&,const Units&);
  friend constexpr Units operator*(const ScalingFactor&,const Units&);
  friend constexpr Units operator/(const Units&,const Units&);
  friend constexpr Units operator/(const Units&,const ScalingFactor&);
  friend constexpr Units operator/(const ScalingFactor&,const Units&);
  friend constexpr Units pow(const Units&,const RationalConstant&);
  friend constexpr Units sqrt(const Units&);

  ScalingFactor                   m_scaling;
  std::array<RationalConstant,7>  m_units;

  std::array<char,UNITS_MAX_STR_LEN> m_string_repr = {'\0'};
};

// === Operators/functions overload === //
//
// Recall: the scaling is a factor, while the 7 rational constants
//         are only exponents. So in u1*u2, multiply the factor, and
//         add the exponents

// --- Comparison --- //

constexpr bool operator==(const Units& lhs, const Units& rhs) {
  // Do not compare names, since N == kg*m/(s*s)
  return lhs.m_scaling==rhs.m_scaling &&
         lhs.m_units[0]==rhs.m_units[0] &&
         lhs.m_units[1]==rhs.m_units[1] &&
         lhs.m_units[2]==rhs.m_units[2] &&
         lhs.m_units[3]==rhs.m_units[3] &&
         lhs.m_units[4]==rhs.m_units[4] &&
         lhs.m_units[5]==rhs.m_units[5] &&
         lhs.m_units[6]==rhs.m_units[6];
}

constexpr bool operator!=(const Units& lhs, const Units& rhs) {
  return !(lhs==rhs);
}

// --- Multiplicaiton --- //
constexpr Units operator*(const Units& lhs, const Units& rhs) {
  return Units (lhs.m_units[0]+rhs.m_units[0],
                lhs.m_units[1]+rhs.m_units[1],
                lhs.m_units[2]+rhs.m_units[2],
                lhs.m_units[3]+rhs.m_units[3],
                lhs.m_units[4]+rhs.m_units[4],
                lhs.m_units[5]+rhs.m_units[5],
                lhs.m_units[6]+rhs.m_units[6],
                lhs.m_scaling*rhs.m_scaling,
                Units::concat_repr(lhs.string_repr(),rhs.string_repr(),'*').data());
}

constexpr Units operator*(const ScalingFactor& lhs, const Units& rhs) {
  using namespace prefixes;
  // If input rhs has its own symbol, we allow prepending a letter to it,
  // to get the usual symbol for 10^n powers
  if (rhs.has_symbol()) {
    Units u (rhs);
    u.m_scaling *= lhs;
    if (lhs==nano) {
      u.m_string_repr = Units::concat_repr("n",u.string_repr(),'\0');
    } else if (lhs==micro) {
      u.m_string_repr = Units::concat_repr("u",u.string_repr(),'\0');
    } else if (lhs==milli) {
      u.m_string_repr = Units::concat_repr("m",u.string_repr(),'\0');
    } else if (lhs==centi) {
      u.m_string_repr = Units::concat_repr("c",u.string_repr(),'\0');
    } else if (lhs==hecto) {
      u.m_string_repr = Units::concat_repr("h",u.string_repr(),'\0');
    } else if (lhs==kilo) {
      u.m_string_repr = Units::concat_repr("k",u.string_repr(),'\0');
    } else if (lhs==mega) {
      u.m_string_repr = Units::concat_repr("M",u.string_repr(),'\0');
    } else if (lhs==giga) {
      u.m_string_repr = Units::concat_repr("G",u.string_repr(),'\0');
    } else {
      return Units(lhs)*rhs;
    }
    return u;
  }
  return Units(lhs)*rhs;
}
constexpr Units operator*(const Units& lhs, const ScalingFactor& rhs) {
  return rhs*lhs;
}
constexpr Units operator*(const RationalConstant& lhs, const Units& rhs) {
  return ScalingFactor(lhs)*rhs;
}
constexpr Units operator*(const Units& lhs, const RationalConstant& rhs) {
  return lhs*ScalingFactor(rhs);
}

// --- Division --- //
constexpr Units operator/(const Units& lhs, const Units& rhs) {
  return Units(lhs.m_units[0]-rhs.m_units[0],
               lhs.m_units[1]-rhs.m_units[1],
               lhs.m_units[2]-rhs.m_units[2],
               lhs.m_units[3]-rhs.m_units[3],
               lhs.m_units[4]-rhs.m_units[4],
               lhs.m_units[5]-rhs.m_units[5],
               lhs.m_units[6]-rhs.m_units[6],
               lhs.m_scaling/rhs.m_scaling,
               Units::concat_repr(lhs.string_repr(),rhs.string_repr(),'/').data());
}
constexpr Units operator/(const Units& lhs, const ScalingFactor& rhs) {
  return lhs/Units(rhs);
}
constexpr Units operator/(const ScalingFactor& lhs, const Units& rhs) {
  return Units(lhs)/rhs;
}
constexpr Units operator/(const RationalConstant& lhs, const Units& rhs) {
  return ScalingFactor(lhs)/rhs;
}
constexpr Units operator/(const Units& lhs, const RationalConstant& rhs) {
  return lhs/ScalingFactor(rhs);
}

// --- Powers and roots --- //
constexpr Units pow(const Units& x, const RationalConstant& p) {
  return Units(x.m_units[0]*p,
               x.m_units[1]*p,
               x.m_units[2]*p,
               x.m_units[3]*p,
               x.m_units[4]*p,
               x.m_units[5]*p,
               x.m_units[6]*p,
               pow(x.m_scaling,p),
               Units::concat_repr(x.string_repr(),p.string_repr<UNITS_MAX_STR_LEN>().data(),'^').data());
}

constexpr Units sqrt(const Units& x) {
  return pow(x,RationalConstant(1,2));
}

inline std::ostream& operator<< (std::ostream& out, const Units& x) {
  out << x.to_string();
  return out;
}

// ================== SHORT NAMES FOR COMMON SI PHYSICAL UNITS =============== //

// Note to developers:
// I added the 'most common' SI units, but I avoided 'Siemens', since the symbol
// is 'S', which is way too close to 's' (seconds).

// === FUNDAMENTAL === //

// Note: no need to pass a string for these, since the default
//       string construction would return the same thing.
constexpr auto m   = Units(1,0,0,0,0,0,0,ScalingFactor::one(),"m");
constexpr auto s   = Units(0,1,0,0,0,0,0,ScalingFactor::one(),"s");
constexpr auto kg  = Units(0,0,1,0,0,0,0,ScalingFactor::one(),"kg");
constexpr auto K   = Units(0,0,0,1,0,0,0,ScalingFactor::one(),"K");
constexpr auto A   = Units(0,0,0,0,1,0,0,ScalingFactor::one(),"A");
constexpr auto mol = Units(0,0,0,0,0,1,0,ScalingFactor::one(),"mol");
constexpr auto cd  = Units(0,0,0,0,0,0,1,ScalingFactor::one(),"cd");

// Nondimensional units
constexpr auto none = Units(ScalingFactor::one(),"1");

// === DERIVED SI UNITS === //

// Thermomechanics
constexpr auto h    = (3600*s).rename("h");       // hour         (time)
constexpr auto day  = (86400*s).rename("day");    // day          (time)
constexpr auto year = (365*day).rename("yr");     // year         (time)
constexpr auto g    = (kg/1000).rename("g");      // gram         (mass)
constexpr auto N    = (kg*m/(s*s)).rename("N");   // newton       (force)
constexpr auto Pa   = (N/(m*m)).rename("Pa");     // pascal       (pressure)
constexpr auto bar  = (100000*Pa).rename("bar");  // bar          (pressure)
constexpr auto atm  = (101325*Pa).rename("atm");  // atmosphere   (pressure)
constexpr auto J    = (N*m).rename("J");          // joule        (energy)
constexpr auto W    = (J/s).rename("W");          // watt         (power)

// Electro-magnetism
constexpr auto C    = (A*s).rename("C");          // coulomb      (charge)
constexpr auto V    = (J/C).rename("V");          // volt         (voltage)
constexpr auto T    = (N/(A*m)).rename("T");      // tesla        (magnetic field)
constexpr auto F    = (C/V).rename("F");          // farad        (capacitance)
constexpr auto Wb   = (V*s).rename("Wb");         // weber        (magnetic flux)
constexpr auto H    = (Wb/A).rename("H");         // henri        (inductance)
constexpr auto Sv   = (J/kg).rename("Sv");        // sievert      (radiation dose)
constexpr auto Hz   = (1/s).rename("Hz");         // hertz        (frequency)

// Angle and solid angle
constexpr auto rad = none.rename("rad");   // radian     (angle)
constexpr auto sr  = none.rename("sr");    // steradian  (solid angle)

// Deprecated
[[deprecated("units::dyn is CGS and deprecated.")]]
constexpr auto dyn  = (N/10000).rename("dyn");    // dyne         (force)
[[deprecated("units::rem is non-SI and deprecated.")]]
constexpr auto rem  = (Sv/100).rename("rem");     // rem          (radiation dose)

} // namespace units

} // namespace ekat

#endif // EKAT_UNITS_HPP
