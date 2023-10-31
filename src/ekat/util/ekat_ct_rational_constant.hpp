#ifndef EKAT_COMPILE_TIME_RATIONAL_CONSTANT_HPP
#define EKAT_COMPILE_TIME_RATIONAL_CONSTANT_HPP

#include <sstream>

#include "ekat/ekat_assert.hpp"

namespace ekat {

namespace rational {

template<bool B, typename T, typename F>
using cond_t = typename std::conditional<B,T,F>::type;

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

template<long long Num, long long Den = 1>
struct RationalConstant {
  using iType = long long;

protected:
  static constexpr iType fix_num (const iType n, const iType d) {
    return d<0 ? fix_num(-n,-d) :
                 n==0 ? n : n / gcd(abs(n),d);
  }
  static constexpr iType fix_den(const iType n, const iType d) {
    return d<0 ? fix_den(-n,-d) : d / gcd(abs(n),d);
  }
public:
  static constexpr iType den = Den==0 ? 0   : fix_den(Num,Den);
  static constexpr iType num = Den==0 ? Num : fix_num(Num,Den);

  static std::string to_string (const Format fmt = Format::Rat) {
    // Note: using std::to_string(double) causes insertion of trailing zeros,
    //       and the insertion of decimal part for integers.
    //       E.g., to_string(1.0/2) leads to 0.5000000
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
        EKAT_ERROR_MSG("Error! Unrecognized format for printing RationalConstant.\n");

    }
    return ss.str();
  }
};

// n/0 is undefined
template<long long Num>
struct RationalConstant<Num,0>;

using one_t = RationalConstant<1>;
using zero_t = RationalConstant<0>;

template<typename R>
using neg_t = RationalConstant<-R::num,R::den>;
template<typename R1, typename R2>
using add_t = RationalConstant<R1::num*R2::den + R1::den*R2::num,R1::den*R2::den>;
template<typename R1, typename R2>
using sub_t = RationalConstant<R1::num*R2::den - R1::den*R2::num,R1::den*R2::den>;
template<typename R1, typename R2>
using mult_t = RationalConstant<R1::num*R2::num,R1::den*R2::den>;
template<typename R>
using square_t = mult_t<R,R>;
template<typename R1, typename R2>
using divide_t = RationalConstant<R1::num*R2::den,R1::den*R2::num>;
template<typename R>
using inv_t = divide_t<one_t,R>;

template<typename R, long long p, bool p_pos>
struct PowImpl;

// 0^0 is undefined
template<long long den>
struct PowImpl<RationalConstant<0,den>,0,true>;

// any other pow of 0 is 0
template<long long den, long long p>
struct PowImpl<RationalConstant<0,den>,p,true>
{
  using type = zero_t;
};

// Any non-zero number to power 0 is one
template<typename R>
struct PowImpl<R,0,true>
{
  using type = one_t;
};

// If p<0, result is 1/R^(-p)
template<typename R, long long p>
struct PowImpl<R,p,false>
{
  using type = PowImpl<inv_t<R>,-p,true>;
};

// Base case, with p>0
template<typename R, long long p>
struct PowImpl<R,p,true>
{
  using R2 = square_t<R>;
  static constexpr bool p_odd = p & 1;
  static constexpr long long pp1 = p>>1;

  using factor = cond_t<p_odd,R,one_t>;
  using type = mult_t<factor,typename PowImpl<R2,pp1,true>::type>;
};

template<typename R, long long p>
using pow_t = typename PowImpl<R,p,(p>=0)>::type;

template<long long n, long long d>
inline std::string to_string (const RationalConstant<n,d>&,
                              const Format fmt = Format::Rat) {
  // Note: using std::to_string(double) causes insertion of trailing zeros,
  //       and the insertion of decimal part for integers.
  //       E.g., to_string(1.0/2) leads to 0.5000000
  std::stringstream ss;
  switch (fmt) {
    case Format::Float:
      ss << static_cast<double>(n)/d;
      break;
    case Format::Rat:
      ss << n;
      if (d!=1) {
        ss << "/" << d;
      }
      break;
    default:
      EKAT_ERROR_MSG("Error! Unrecognized format for printing RationalConstant.\n");

  }
  return ss.str();
}

} // namespace rational

} // namespace ekat

template<long long n, long long d>
inline std::ostream&
operator<< (std::ostream& out, const ekat::rational::RationalConstant<n,d>& rat) {
  out << to_string(rat);
  return out;
}

#endif // EKAT_COMPILE_TIME_RATIONAL_CONSTANT_HPP
