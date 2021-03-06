#ifndef EKAT_UNITS_HPP
#define EKAT_UNITS_HPP

#include "ekat/util/ekat_rational_constant.hpp"
#include "ekat/util/ekat_scaling_factor.hpp"
#include "ekat/util/ekat_string_utils.hpp"

namespace ekat
{

namespace units
{

constexpr int NUM_BASIC_UNITS = 7;
constexpr const char* BASIC_UNITS_SYMBOLS[7] = {"m", "s", "kg", "K", "A", "mol", "cd"};

/*
 *  Units: a class to store physical units in terms of fundamental ones
 *
 *  Units is morally storing 8 numbers:
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
 *  Note: we do *not* overload operator^. Although it would be nice to write
 *            auto my_units = m^2 / s^2;
 *        it would be very bug prone, since ^ has lower precedence than + - * /.
 *        Sure, using parentheses makes it safe, but then you may as well write
 *            auto my_units = pow(m,2) / pow(s,2);
 *
 *  Note: there should be no need to store a string, since we can build a string
 *        on the fly when printing the units. However, some apps might prefer to
 *        use common strings for certain units. E.g., for 'auto N = kg*m/(s*s)'
 *        it might be preferable to print "N" rather than "kg*m/(s*s)".
 */

class Units {
public:

  // No default
  Units () = delete;

  // Construct a non-dimensional quantity
  Units (const ScalingFactor& scaling)
   : Units(0,0,0,0,0,0,0,scaling)
  {
    // Nothing to do here
  }

  // Construct a general quantity
  Units (const RationalConstant& lengthExp,
         const RationalConstant& timeExp,
         const RationalConstant& massExp,
         const RationalConstant& temperatureExp,
         const RationalConstant& currentExp,
         const RationalConstant& amountExp,
         const RationalConstant& luminousIntensityExp,
         const ScalingFactor& scalingFactor = RationalConstant::one())
   : m_scaling {scalingFactor}
   , m_units {lengthExp,timeExp,massExp,temperatureExp,currentExp,amountExp,luminousIntensityExp}
   , m_string {""}
  {
    // Nothing to do here
  }

  Units (const Units&) = default;

  static Units nondimensional () {
    return Units(RationalConstant::one());
  }

  void set_string (const std::string& str) {
    m_string = str;
  }

  std::string get_string () const {
    return m_string=="" ? to_string(*this) : m_string;
  }

  bool is_dimensionless () const {
    return m_units[0].num==0 &&
           m_units[1].num==0 &&
           m_units[2].num==0 &&
           m_units[3].num==0 &&
           m_units[4].num==0 &&
           m_units[5].num==0 &&
           m_units[6].num==0;
  }

private:

  friend bool operator==(const Units&,const Units&);

  friend Units operator*(const Units&,const Units&);
  friend Units operator*(const ScalingFactor&,const Units&);
  friend Units operator/(const Units&,const Units&);
  friend Units operator/(const Units&,const ScalingFactor&);
  friend Units operator/(const ScalingFactor&,const Units&);
  friend Units pow(const Units&,const RationalConstant&);
  friend Units sqrt(const Units&);
  friend Units root(const Units&,const int);

  friend std::string to_string(const Units&);

  const ScalingFactor     m_scaling;
  const RationalConstant  m_units[7];

  std::string             m_string;
};

// === Operators/functions overload === //
//
// Recall: the scaling is a factor, while the 7 rational constants
//         are only exponents. So in u1*u2, multiply the factor, and
//         add the exponents

// --- Comparison --- //

inline bool operator==(const Units& lhs, const Units& rhs) {
  return lhs.m_scaling==rhs.m_scaling &&
         lhs.m_units[0]==rhs.m_units[0] &&
         lhs.m_units[1]==rhs.m_units[1] &&
         lhs.m_units[2]==rhs.m_units[2] &&
         lhs.m_units[3]==rhs.m_units[3] &&
         lhs.m_units[4]==rhs.m_units[4] &&
         lhs.m_units[5]==rhs.m_units[5] &&
         lhs.m_units[6]==rhs.m_units[6];
}

inline bool operator!=(const Units& lhs, const Units& rhs) {
  return !(lhs==rhs);
}

// --- Multiplicaiton --- //
inline Units operator*(const Units& lhs, const Units& rhs) {
  return Units(lhs.m_units[0]+rhs.m_units[0],
               lhs.m_units[1]+rhs.m_units[1],
               lhs.m_units[2]+rhs.m_units[2],
               lhs.m_units[3]+rhs.m_units[3],
               lhs.m_units[4]+rhs.m_units[4],
               lhs.m_units[5]+rhs.m_units[5],
               lhs.m_units[6]+rhs.m_units[6],
               lhs.m_scaling*rhs.m_scaling);
}
inline Units operator*(const ScalingFactor& lhs, const Units& rhs) {
  return Units(rhs.m_units[0],
               rhs.m_units[1],
               rhs.m_units[2],
               rhs.m_units[3],
               rhs.m_units[4],
               rhs.m_units[5],
               rhs.m_units[6],
               lhs*rhs.m_scaling);
}
inline Units operator*(const Units& lhs, const ScalingFactor& rhs) {
  return rhs*lhs;
}
inline Units operator*(const RationalConstant& lhs, const Units& rhs) {
  return ScalingFactor(lhs)*rhs;
}
inline Units operator*(const Units& lhs, const RationalConstant& rhs) {
  return lhs*ScalingFactor(rhs);
}

// --- Division --- //
inline Units operator/(const Units& lhs, const Units& rhs) {
  return Units(lhs.m_units[0]-rhs.m_units[0],
               lhs.m_units[1]-rhs.m_units[1],
               lhs.m_units[2]-rhs.m_units[2],
               lhs.m_units[3]-rhs.m_units[3],
               lhs.m_units[4]-rhs.m_units[4],
               lhs.m_units[5]-rhs.m_units[5],
               lhs.m_units[6]-rhs.m_units[6],
               lhs.m_scaling/rhs.m_scaling);
}
inline Units operator/(const Units& lhs, const ScalingFactor& rhs) {
  return Units(lhs.m_units[0],
               lhs.m_units[1],
               lhs.m_units[2],
               lhs.m_units[3],
               lhs.m_units[4],
               lhs.m_units[5],
               lhs.m_units[6],
               lhs.m_scaling/rhs);
}
inline Units operator/(const ScalingFactor& lhs, const Units& rhs) {
  return Units(-rhs.m_units[0],
               -rhs.m_units[1],
               -rhs.m_units[2],
               -rhs.m_units[3],
               -rhs.m_units[4],
               -rhs.m_units[5],
               -rhs.m_units[6],
               lhs/rhs.m_scaling);
}
inline Units operator/(const RationalConstant& lhs, const Units& rhs) {
  return ScalingFactor(lhs)/rhs;
}
inline Units operator/(const Units& lhs, const RationalConstant& rhs) {
  return lhs/ScalingFactor(rhs);
}

// --- Powers and roots --- //

inline Units pow(const Units& x, const RationalConstant& p) {
  return Units(x.m_units[0]*p,
               x.m_units[1]*p,
               x.m_units[2]*p,
               x.m_units[3]*p,
               x.m_units[4]*p,
               x.m_units[5]*p,
               x.m_units[6]*p,
               pow(x.m_scaling,p));
}

inline Units sqrt(const Units& x) {
  return Units(x.m_units[0] / 2,
               x.m_units[1] / 2,
               x.m_units[2] / 2,
               x.m_units[3] / 2,
               x.m_units[4] / 2,
               x.m_units[5] / 2,
               x.m_units[6] / 2,
               pow(x.m_scaling,RationalConstant(1,2)));
}

inline std::string to_string(const Units& x) {
  std::string s;
  int num_non_trivial = 0;
  for (int i=0; i<NUM_BASIC_UNITS; ++i) {
    if (x.m_units[i].num==0) {
      continue;
    }
    ++num_non_trivial;
    s += BASIC_UNITS_SYMBOLS[i];
    if (x.m_units[i]!=RationalConstant::one()) {
      s += "^" + to_string(x.m_units[i],Format::Rat);
    }
    s += " ";
  }

  // Prepend the scaling only if it's not one, or if this is a dimensionless unit
  if (x.m_scaling!=ScalingFactor::one() || num_non_trivial==0) {
    s = to_string(x.m_scaling) + " " + s;
  }

  // Remove leading/trailing whitespaces
  return trim(s);
}

inline std::ostream& operator<< (std::ostream& out, const Units& x) {
  out << to_string(x);
  return out;
}

// ================== SHORT NAMES FOR COMMON PHYSICAL UNITS =============== //

// Note to developers:
// I added the 'most common' units. I avoided 'Siemes', since
// the symbol is 'S', which is way too close to 's' (seconds).
// TODO: should we add 'common' scaled ones, such as km=kilo*m?

// === FUNDAMENTAL === //

const Units m   = Units(1,0,0,0,0,0,0);
const Units s   = Units(0,1,0,0,0,0,0);
const Units kg  = Units(0,0,1,0,0,0,0);
const Units K   = Units(0,0,0,1,0,0,0);
const Units A   = Units(0,0,0,0,1,0,0);
const Units mol = Units(0,0,0,0,0,1,0);
const Units cd  = Units(0,0,0,0,0,0,1);

// === DERIVED === //

// Thermomechanics
const Units day  = 86400*s;      // day          (time)
const Units year = 365*day;      // year         (time)
const Units g    = kg/1000;      // gram         (mass)
const Units N    = kg*m/(s*s);   // newton       (force)
const Units dyn  = N/(10000);    // dyne         (force)
const Units Pa   = N/(m*m);      // pascal       (pressure)
const Units bar  = 10000*Pa;     // bar          (pressure)
const Units atm  = 101325*Pa;    // atmosphere   (pressure)
const Units J    = N*m;          // joule        (energy)
const Units W    = J/s;          // watt         (power)

// Electro-magnetism
const Units C    = A*s;          // coulomb      (charge)
const Units V    = J/C;          // volt         (voltage)
const Units T    = N/(A*m);      // tesla        (magnetic field)
const Units F    = C/V;          // farad        (capacitance)
const Units Wb   = V*s;          // weber        (magnetic flux)
const Units H    = Wb/A;         // henri        (inductance)
const Units Sv   = J/kg;         // sievert      (radiation dose)
const Units rem  = Sv/100;       // rem          (radiation dose)
const Units Hz   = 1/s;          // hertz        (frequency)

} // namespace units

} // namespace ekat

#endif // EKAT_UNITS_HPP
