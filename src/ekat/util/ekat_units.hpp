#ifndef EKAT_UNITS_HPP
#define EKAT_UNITS_HPP

#include "ekat/util/ekat_rational_constant.hpp"
#include "ekat/util/ekat_scaling_factor.hpp"
#include "ekat/util/ekat_string_utils.hpp"

#include <array>
#include <limits>
#include <string_view>

namespace ekat
{

namespace units
{

constexpr int UNITS_MAX_STR_LEN = 128;
constexpr int NUM_BASIC_UNITS = 7;
constexpr std::array<char,UNITS_MAX_STR_LEN> BASIC_UNITS_SYMBOLS[7] = {
  {"m"}, {"s"}, {"kg"}, {"K"}, {"A"}, {"mol"}, {"cd"}
};

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
   : Units(0,0,0,0,0,0,0,scaling,scaling.string_repr<UNITS_MAX_STR_LEN>())
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
                   const std::array<char,UNITS_MAX_STR_LEN>& n = {'\0'})
   : m_scaling {scalingFactor}
   , m_units {lengthExp,timeExp,massExp,temperatureExp,currentExp,amountExp,luminousIntensityExp}
   , m_string_repr(n)
  {
    // Nothing to do here
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

  static constexpr Units nondimensional () {
    Units u(ScalingFactor::one());
    u.m_string_repr[0] = '1';
    return u;
  }
  static constexpr Units invalid () {
    constexpr auto infty = std::numeric_limits<RationalConstant::iType>::max();
    return ScalingFactor(-infty)*nondimensional();
  }

  std::string get_si_string () const {
    std::string s;
    for (int i=0; i<NUM_BASIC_UNITS; ++i) {
      if (m_units[i].num==0) {
        continue;
      }
      s += std::string(BASIC_UNITS_SYMBOLS[i].data());
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

private:
  constexpr const std::array<char,UNITS_MAX_STR_LEN>& string_repr () const {
    return m_string_repr;
  }

  // Returns true if the unit is composite (i.e., does not have a one-word symbol,
  // but instead is a compound of symbols, such as a*b/c)
  static constexpr bool composite (const std::array<char,UNITS_MAX_STR_LEN>& sv) {
    for (const auto& c : sv) {
      if (c=='*' or c=='/' or c=='^')
        return true;
      if (c=='\0')
        return false;
    }
    return false;
  };

  static constexpr std::array<char,UNITS_MAX_STR_LEN>
  concat_repr (const std::array<char,UNITS_MAX_STR_LEN>& lhs,
               const std::array<char,UNITS_MAX_STR_LEN>& rhs,
               const char sep)
  {
    std::array<char,UNITS_MAX_STR_LEN> out = {'\0'};
    const auto comp1 = composite(lhs);
    const auto comp2 = composite(rhs);
    int size1 = 0;
    for (auto c : lhs) {
      if (c=='\0') break;
      ++size1;
    }
    int size2 = 0;
    for (auto c : rhs) {
      if (c=='\0') break;
      ++size2;
    }
    if (size1==0) {
      return rhs;
    } else if (size2==0) {
      return lhs;
    }

    assert ( size1 + size2 + (sep == '\0' ? 0 : 1) + (comp1 ? 2 : 0) + (comp2 ? 2 : 0) < UNITS_MAX_STR_LEN );

    int pos=0;
    if (comp1) {
      out[pos++]='(';
    }
    for (int i=0; i<size1; ++i) {
      out [pos++] = lhs[i];
    }
    if (comp1) {
      out[pos++] = ')';
    }
    if (sep!='\0') {
      out[pos++] = sep;
    }
    if (comp2) {
      out[pos++]='(';
    }
    for (int i=0; i<size2; ++i) {
      out [pos++] = rhs[i];
    }
    if (comp2) {
      out[pos++] = ')';
    }
    out[pos] = '\0';
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
                Units::concat_repr(lhs.string_repr(),rhs.string_repr(),'*'));
}

constexpr Units operator*(const ScalingFactor& lhs, const Units& rhs) {
  using namespace prefixes;
  // If input rhs has its own symbol, we allow prepending a letter to it,
  // to get the usual symbol for 10^n powers
  if (rhs.has_symbol()) {
    Units u (rhs);
    u.m_scaling *= lhs;
    if (lhs==nano) {
      u.m_string_repr = Units::concat_repr({'n'},u.string_repr(),'\0');
    } else if (lhs==micro) {
      u.m_string_repr = Units::concat_repr({'u'},u.string_repr(),'\0');
    } else if (lhs==milli) {
      u.m_string_repr = Units::concat_repr({'m'},u.string_repr(),'\0');
    } else if (lhs==centi) {
      u.m_string_repr = Units::concat_repr({'c'},u.string_repr(),'\0');
    } else if (lhs==hecto) {
      u.m_string_repr = Units::concat_repr({'h'},u.string_repr(),'\0');
    } else if (lhs==kilo) {
      u.m_string_repr = Units::concat_repr({'k'},u.string_repr(),'\0');
    } else if (lhs==mega) {
      u.m_string_repr = Units::concat_repr({'M'},u.string_repr(),'\0');
    } else if (lhs==giga) {
      u.m_string_repr = Units::concat_repr({'G'},u.string_repr(),'\0');
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
               Units::concat_repr(lhs.string_repr(),rhs.string_repr(),'/'));
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
               Units::concat_repr(x.string_repr(),p.string_repr<UNITS_MAX_STR_LEN>(),'^'));
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
constexpr Units m   = Units(1,0,0,0,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[0]);
constexpr Units s   = Units(0,1,0,0,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[1]);
constexpr Units kg  = Units(0,0,1,0,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[2]);
constexpr Units K   = Units(0,0,0,1,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[3]);
constexpr Units A   = Units(0,0,0,0,1,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[4]);
constexpr Units mol = Units(0,0,0,0,0,1,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[5]);
constexpr Units cd  = Units(0,0,0,0,0,0,1,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[6]);

// === DERIVED SI UNITS === //

// Thermomechanics
constexpr Units h    = Units(3600*s,"h");
constexpr Units day  = 86400*s;                   // day          (time)
constexpr Units year = 365*day;                   // year         (time)
constexpr Units g    = Units(kg/1000,"g");        // gram         (mass)
constexpr Units N    = Units(kg*m/(s*s),"N");     // newton       (force)
constexpr Units dyn  = N/(10000);                 // dyne         (force)
constexpr Units Pa   = Units(N/(m*m),"Pa");       // pascal       (pressure)
constexpr Units bar  = Units(100000*Pa,"bar");    // bar          (pressure)
constexpr Units atm  = Units(101325*Pa,"atm");    // atmosphere   (pressure)
constexpr Units J    = Units(N*m,"J");            // joule        (energy)
constexpr Units W    = Units(J/s,"W");            // watt         (power)

// Electro-magnetism
constexpr Units C    = Units(A*s,"C");            // coulomb      (charge)
constexpr Units V    = Units(J/C,"V");            // volt         (voltage)
constexpr Units T    = Units(N/(A*m),"T");        // tesla        (magnetic field)
constexpr Units F    = Units(C/V,"F");            // farad        (capacitance)
constexpr Units Wb   = Units(V*s,"Wb");           // weber        (magnetic flux)
constexpr Units H    = Units(Wb/A,"H");           // henri        (inductance)
constexpr Units Sv   = Units(J/kg,"Sv");          // sievert      (radiation dose)
constexpr Units rem  = Units(Sv/100,"rem");       // rem          (radiation dose)
constexpr Units Hz   = Units(1/s,"Hz");           // hertz        (frequency)

} // namespace units

} // namespace ekat

#endif // EKAT_UNITS_HPP
