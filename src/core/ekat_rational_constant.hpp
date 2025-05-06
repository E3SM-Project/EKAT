#ifndef EKAT_RATIONAL_CONSTANT_HPP
#define EKAT_RATIONAL_CONSTANT_HPP

#include "ekat_assert.hpp"

#include <stdexcept>
#include <sstream>
#include <array>

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

  iType num;
  iType den;

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
   : RationalConstant (n,static_cast<IntType>(1))
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
  constexpr RationalConstant& operator= (const RationalConstant&) = default;

  constexpr RationalConstant operator- () const {
    return RationalConstant(-num,den);
  }

  static constexpr RationalConstant one () { return RationalConstant(1); }
  static constexpr RationalConstant zero () { return RationalConstant(0); }

  template<int N>
  constexpr std::array<char,N> string_repr () const
  {
    std::array<char,N> s = {'\0'};
    int pos = 0;
    int nd_num = ndigits(num);
    int nd_den = ndigits(den);

    assert (N>(nd_num+nd_den+(num<0 ? 1 : 0)));

    if (num<0) {
      s[pos++] = '-';
    }
    int absnum = num<0 ? -num : num;
    for (int i=(nd_num-1); i>=0; --i) {
      s[pos++] = digit2char(get_digit(absnum,i));
    }

    if (den!=1) {
      s[pos++] = '/';
      for (int i=(nd_den-1); i>=0; --i) {
        s[pos++] = digit2char(get_digit(den,i));
      }
    }

    return s;
  }

  std::string to_string (const Format fmt = Format::Rat) const 
  {
    std::stringstream ss;
    switch (fmt) {
      case Format::Float:
        ss << static_cast<double>(num)/den;
        break;
      case Format::Rat:
        ss << num;
        if (den!=1) {
          ss << "/" << den;
        }
        break;
      default:
        EKAT_REQUIRE_MSG(false,"Error! Unrecognized format for printing RationalConstant.\n");

    }
    return ss.str();
  }

private:
  static constexpr char digit2char (int n) {
    if (n==0) return '0';
    else if (n==1) return '1';
    else if (n==2) return '2';
    else if (n==3) return '3';
    else if (n==4) return '4';
    else if (n==5) return '5';
    else if (n==6) return '6';
    else if (n==7) return '7';
    else if (n==8) return '8';
    else return '9';
  }

  static constexpr int ndigits (iType n) {
    return n<10 ? 1 : 1 + ndigits(n/10);
  }

  static constexpr int get_digit(int n, int ten_pow_exp) {
    int ten_pow = 1;
    for (int i=0; i<ten_pow_exp; ++i) {
      ten_pow *= 10;
    }

    return (n / ten_pow) % 10;
  }

  // These two are used to help reduce a/b to lowest terms
  static constexpr iType fix_num(const iType n, const iType d) {
    if (d==0)
      throw std::invalid_argument("[RationalConstant] Cannot divide by 0");
    return n==0 ? n : n / gcd(abs(n),abs(d));
  }

  static constexpr iType fix_den(const iType n, const iType d) {
    if (d==0)
      throw std::invalid_argument("[RationalConstant] Cannot divide by 0");
    return d<0 ? fix_den(n,-d) : d / gcd(abs(n),abs(d));
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
constexpr
typename std::enable_if<std::is_integral<IntType>::value,RationalConstant>::type
pow (const RationalConstant& x, const IntType p) {
  // Three main cases:
  //  - p<0: compute the -p power of 1/x
  //  - p=0: base case, return 1 if x!=0, throw if x==0
  //  - recursion step: x^p = x * x^{p-1}
  // Note: recall that expressions like "blah1, blah2" executes both blah1 and blah2 and return blah2.
  if (p<0) {
    return pow(1/x, -p);
  } else if (p==0) {
    if (x.num==0)
      throw std::invalid_argument("[RationalConstant] Cannot compute 0^0.");

    return RationalConstant::one();
  } else {
    return p&1!=0 ? x*pow(x*x,p>>1) : pow(x*x,p>>1);
  }
}

inline std::ostream& operator<< (std::ostream& out, const RationalConstant& rat) {
  out << rat.to_string();
  return out;
}

} // namespace ekat

#endif // EKAT_RATIONAL_CONSTANT_HPP
