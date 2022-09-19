#ifndef EKAT_UNITS_HPP
#define EKAT_UNITS_HPP

#include "ekat/util/ekat_rational_constant.hpp"
#include "ekat/util/ekat_scaling_factor.hpp"
#include "ekat/util/ekat_string_utils.hpp"

#include <array>
#include <limits>

namespace ekat
{

namespace units
{

constexpr int NUM_BASIC_UNITS = 7;
constexpr const char* BASIC_UNITS_SYMBOLS[7] = {"m", "s", "kg", "K", "A", "mol", "cd"};

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
 *        it would be very bug prone, since ^ has lower precedence than + - * /.
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
   : Units(0,0,0,0,0,0,0,scaling)
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
         const char* name = nullptr)
   : m_scaling {scalingFactor}
   , m_units {lengthExp,timeExp,massExp,temperatureExp,currentExp,amountExp,luminousIntensityExp}
   , m_name {name}
  {
    // Nothing to do here
  }
  constexpr Units (const Units& rhs, const char* n)
   : m_scaling (rhs.m_scaling)
   , m_units   (rhs.m_units)
   , m_name    (n)
  {
    // Nothing to do here
  }

  constexpr Units (const Units&) = default;

  constexpr Units& operator= (const Units&) = default;

  static constexpr Units nondimensional () {
    return Units(ScalingFactor::one());
  }
  static constexpr Units invalid () {
    constexpr auto infty = std::numeric_limits<RationalConstant::iType>::max();
    return ScalingFactor(-infty)*nondimensional();
  }

  void set_string (const char* name) {
    m_name = name;
  }

  std::string get_string () const {
    return m_name==nullptr ? to_string(*this) : m_name;
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

private:

  friend constexpr bool operator==(const Units&,const Units&);

  friend constexpr Units operator*(const Units&,const Units&);
  friend constexpr Units operator*(const ScalingFactor&,const Units&);
  friend constexpr Units operator/(const Units&,const Units&);
  friend constexpr Units operator/(const Units&,const ScalingFactor&);
  friend constexpr Units operator/(const ScalingFactor&,const Units&);
  friend constexpr Units pow(const Units&,const RationalConstant&);
  friend constexpr Units sqrt(const Units&);
  friend constexpr Units root(const Units&,const int);

  friend std::string to_string(const Units&);

  ScalingFactor                   m_scaling;
  std::array<RationalConstant,7>  m_units;

  const char*                     m_name;
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
  return Units(lhs.m_units[0]+rhs.m_units[0],
               lhs.m_units[1]+rhs.m_units[1],
               lhs.m_units[2]+rhs.m_units[2],
               lhs.m_units[3]+rhs.m_units[3],
               lhs.m_units[4]+rhs.m_units[4],
               lhs.m_units[5]+rhs.m_units[5],
               lhs.m_units[6]+rhs.m_units[6],
               lhs.m_scaling*rhs.m_scaling);
}
constexpr Units operator*(const ScalingFactor& lhs, const Units& rhs) {
  return Units(rhs.m_units[0],
               rhs.m_units[1],
               rhs.m_units[2],
               rhs.m_units[3],
               rhs.m_units[4],
               rhs.m_units[5],
               rhs.m_units[6],
               lhs*rhs.m_scaling);
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
               lhs.m_scaling/rhs.m_scaling);
}
constexpr Units operator/(const Units& lhs, const ScalingFactor& rhs) {
  return Units(lhs.m_units[0],
               lhs.m_units[1],
               lhs.m_units[2],
               lhs.m_units[3],
               lhs.m_units[4],
               lhs.m_units[5],
               lhs.m_units[6],
               lhs.m_scaling/rhs);
}
constexpr Units operator/(const ScalingFactor& lhs, const Units& rhs) {
  return Units(-rhs.m_units[0],
               -rhs.m_units[1],
               -rhs.m_units[2],
               -rhs.m_units[3],
               -rhs.m_units[4],
               -rhs.m_units[5],
               -rhs.m_units[6],
               lhs/rhs.m_scaling);
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
               pow(x.m_scaling,p));
}

constexpr Units sqrt(const Units& x) {
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

// Note: no need to pass a string for these, since the default
//       string construction would return the same thing.
constexpr Units m   = Units(1,0,0,0,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[0]);
constexpr Units s   = Units(0,1,0,0,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[1]);
constexpr Units kg  = Units(0,0,1,0,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[2]);
constexpr Units K   = Units(0,0,0,1,0,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[3]);
constexpr Units A   = Units(0,0,0,0,1,0,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[4]);
constexpr Units mol = Units(0,0,0,0,0,1,0,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[5]);
constexpr Units cd  = Units(0,0,0,0,0,0,1,ScalingFactor::one(),BASIC_UNITS_SYMBOLS[6]);

// === DERIVED === //

// Thermomechanics
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
