#ifndef EKAT_RATIONAL_CONSTANT_HPP
#define EKAT_RATIONAL_CONSTANT_HPP

#include <stdexcept>
#include <sstream>

#include "ekat/ekat_assert.hpp"

namespace ekat {

enum class Format {
  Float,
  Rat
};

template<typename iType>
constexpr iType abs (const iType a) {
  return a>=0 ? a : -a;
}

template<typename iType>
constexpr iType gcd (const iType a, const iType b) {
  return b==0
          ? a
          : gcd(b, a % b);
}

// Elementary representation of a rational number
// Note: we do not allow construction from a double (even if
//       the double is a simple fraction, like 1.5), since
//       its implementation (with correct checks and safeguards)
//       would probably require too much effort, especially
//       considered the scope of the usage of this class

struct RationalConstant {

  using iType = long long;

  const iType num;
  const iType den;

  // No default
  RationalConstant () = delete;

  // No construction from float/double
  RationalConstant (const double x) = delete;
  RationalConstant (const float x) = delete;

  // Construction from a single int N, means N/1
  // Note: do *not* make this explicit, so we can do stuff like 1/a, with a RationalConstant
  template<typename IntType,
           typename = typename std::enable_if<std::is_integral<IntType>::value>::type>
  constexpr RationalConstant (const IntType n)
   : RationalConstant (n,1)
  {
    // Nothing to do here
  }

  template<typename IntType,
           typename = typename std::enable_if<std::is_integral<IntType>::value>::type>
  constexpr RationalConstant (const IntType n, const IntType d)
   : num(fix_num(n,d))
   , den(fix_den(n,d))
  {
    // Nothing to do here
  }
  constexpr RationalConstant (const RationalConstant&) = default;

  constexpr RationalConstant operator- () const {
    return RationalConstant(-num,den);
  }

  static constexpr RationalConstant one () { return RationalConstant(1); }
  static constexpr RationalConstant zero () { return RationalConstant(0); }

private:

  // These two are used to help reduce a/b to lowest terms
  static constexpr iType fix_num(const iType n, const iType d) {
    return CONSTEXPR_ASSERT(d!=0),
             n==0 ? n : n / gcd(abs(n),abs(d));
  }

  static constexpr iType fix_den(const iType n, const iType d) {
    return CONSTEXPR_ASSERT(d!=0),
            d<0 ? fix_den(n,-d) : d / gcd(abs(n),abs(d));
  }
};

constexpr bool operator== (const RationalConstant& lhs, const RationalConstant& rhs) {
  // Cross multiply, in case somehow (should not be possible, unless
  // someone changed the implementation) someone managed to get lhs and/or
  // rhs to not be already reduced to lower terms
  return lhs.num*rhs.den==lhs.den*rhs.num;
}

constexpr bool operator!= (const RationalConstant& lhs, const RationalConstant& rhs) {
  return !(lhs==rhs);
}

constexpr RationalConstant operator+ (const RationalConstant& lhs, const RationalConstant& rhs) {
  return RationalConstant (lhs.num*rhs.den + lhs.den*rhs.num,
                           lhs.den*rhs.den);
}

constexpr RationalConstant operator- (const RationalConstant& lhs, const RationalConstant& rhs) {
  return RationalConstant (lhs.num*rhs.den - lhs.den*rhs.num,
                           lhs.den*rhs.den);
}

constexpr RationalConstant operator* (const RationalConstant& lhs, const RationalConstant& rhs) {
  return RationalConstant (lhs.num*rhs.num,
                           lhs.den*rhs.den);
}

constexpr RationalConstant operator/ (const RationalConstant& lhs, const RationalConstant& rhs) {
  return RationalConstant (lhs.num*rhs.den,
                           lhs.den*rhs.num);
}

// WARNING! If exp>>1, this generates a huge recursion. Use carefully!!
template<typename IntType>
inline constexpr
typename std::enable_if<std::is_integral<IntType>::value,RationalConstant>::type
pow (const RationalConstant& x, const IntType p) {
  // Three main cases:
  //  - p<0: compute the -p power of 1/x
  //  - p=0: base case, return 1 if x!=0, throw if x==0
  //  - recursion step: x^p = x * x^{p-1}
  // Note: recall that expressions like "blah1, blah2" executes both blah1 and blah2 and return blah2.
  return p<0 ? pow(1/x,-p)
             : (p==0 ? CONSTEXPR_ASSERT(x.num!=0), RationalConstant::one()
                     : ( (p&1)!=0 ? x*pow(x*x,p>>1) : pow(x*x,p>>1)));
}

inline std::string to_string (const RationalConstant& rat, const Format fmt = Format::Rat) {
  // Note: using std::to_string(double) causes insertion of trailing zeros,
  //       and the insertion of decimal part for integers.
  //       E.g., to_string(1.0/2) leads to 0.5000000
  std::stringstream ss;
  switch (fmt) {
    case Format::Float:
      ss << static_cast<double>(rat.num)/rat.den;
      break;
    case Format::Rat:
      ss << rat.num;
      if (rat.den!=1) {
        ss << "/" << rat.den;
      }
      break;
    default:
      EKAT_REQUIRE_MSG(false,"Error! Unrecognized format for printing RationalConstant.\n");

  }
  return ss.str();
}

inline std::ostream& operator<< (std::ostream& out, const RationalConstant& rat) {
  out << to_string(rat);
  return out;
}

} // namespace ekat

#endif // EKAT_RATIONAL_CONSTANT_HPP
