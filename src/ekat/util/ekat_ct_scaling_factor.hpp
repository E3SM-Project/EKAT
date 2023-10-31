#ifndef EKAT_COMPILE_TIME_SCALING_FACTOR_HPP
#define EKAT_COMPILE_TIME_SCALING_FACTOR_HPP

#include <iostream>
#include "ekat/util/ekat_ct_rational_constant.hpp"

#include <array>

namespace ekat
{

namespace scaling {

// Base case: valid
template<bool zero_base, bool zero_exp, bool neg_base, bool even_exp_den>
struct ScalingFactorValid : std::true_type {};

// 0^0: invalid
template<bool neg_base, bool even_exp_den>
struct ScalingFactorValid<true,true,neg_base,even_exp_den> : std::false_type {};

// positive root of neg base: invalid
template<bool zero_base, bool zero_exp>
struct ScalingFactorValid<zero_base, zero_exp, true, true> : std::false_type {};
// ----------------------------- //

template<typename Base, typename Exp>
struct ScalingFactor;

template<long long base_num, long long base_den,
         long long exp_num,  long long exp_den>
struct ScalingFactor<rational::RationalConstant<base_num,base_den>,
                     rational::RationalConstant<exp_num,exp_den>>
  : ScalingFactorValid<base_num==0,exp_num==0,base_num<0,exp_den & 2>
{
  using base = rational::RationalConstant<base_num,base_den>;
  using exp  = rational::RationalConstant<exp_num,exp_den>;

  static std::string to_string (const rational::Format exp_fmt = rational::Format::Rat) {
    std::string s = base::to_string(rational::Format::Float);
    if (not std::is_same<exp,rational::one_t>::value) {
      s +=  "^" + exp::to_string(exp_fmt);
    }
    return s;
  }
};

template<long long base_num, long long base_den,
         long long exp_num,  long long exp_den>
using scaling_t = ScalingFactor<rational::RationalConstant<base_num,base_den>,
                                rational::RationalConstant<exp_num,exp_den>>;

// 0^0 is undefined
template<long long bden, long long eden>
struct ScalingFactor<rational::RationalConstant<0,bden>,
                     rational::RationalConstant<0,eden>>;

template<typename T1, typename T2>
struct MultT;

// ScalingFactor * RationalConstant
template<long long sf_base_num, long long sf_base_den,
         long long sf_exp_num,  long long sf_exp_den,
         long long rc_num,      long long rc_den>
struct MultT<ScalingFactor<rational::RationalConstant<sf_base_num,sf_base_den>,
                           rational::RationalConstant<sf_exp_num, sf_exp_den >
                          >,
             rational::RationalConstant<rc_num,rc_den>
            >
 : MultT<ScalingFactor<rational::RationalConstant<sf_base_num,sf_base_den>,
                       rational::RationalConstant<sf_exp_num, sf_exp_den >
                      >,
         ScalingFactor<rational::RationalConstant<rc_num,rc_den>,
                       rational::one_t
                      >
        > {};

// RationalConstant * ScalingFactor
template<long long sf_base_num, long long sf_base_den,
         long long sf_exp_num,  long long sf_exp_den,
         long long rc_num,  long long rc_den>
struct MultT<rational::RationalConstant<rc_num,rc_den>,
             ScalingFactor<rational::RationalConstant<sf_base_num,sf_base_den>,
                           rational::RationalConstant<sf_exp_num, sf_exp_den >
                          >
            >
 : MultT<ScalingFactor<rational::RationalConstant<rc_num,rc_den>,
                       rational::one_t>,
         ScalingFactor<rational::RationalConstant<sf_base_num,sf_base_den>,
                       rational::RationalConstant<sf_exp_num, sf_exp_den >>
        > {};

// Same base
template<long long base_num,     long long base_den,
         long long lhs_exp_num,  long long lhs_exp_den,
         long long rhs_exp_num,  long long rhs_exp_den>
struct MultT<ScalingFactor<rational::RationalConstant<base_num,base_den>,
                           rational::RationalConstant<lhs_exp_num, lhs_exp_den >
                          >,
             ScalingFactor<rational::RationalConstant<base_num,base_den>,
                           rational::RationalConstant<rhs_exp_num, rhs_exp_den >
                          >
            >
{
  // Same base, possibly different exponents
  using base = rational::RationalConstant<base_num,base_den>;
  using lhs_exp = rational::RationalConstant<lhs_exp_num, lhs_exp_den>;
  using rhs_exp = rational::RationalConstant<rhs_exp_num, rhs_exp_den>;

  using type = ScalingFactor<base,rational::add_t<lhs_exp,rhs_exp>>;
};

// Same exp
template<long long exp_num,     long long exp_den,
         long long lhs_base_num,  long long lhs_base_den,
         long long rhs_base_num,  long long rhs_base_den>
struct MultT<ScalingFactor<rational::RationalConstant<lhs_base_num,lhs_base_den>,
                           rational::RationalConstant<exp_num, exp_den >
                          >,
             ScalingFactor<rational::RationalConstant<rhs_base_num,rhs_base_den>,
                           rational::RationalConstant<exp_num, exp_den >
                          >
            >
{
  // Same exp, possibly different bases
  using exp = rational::RationalConstant<exp_num,exp_den>;
  using lhs_base = rational::RationalConstant<lhs_base_num, lhs_base_den>;
  using rhs_base = rational::RationalConstant<rhs_base_num, rhs_base_den>;

  using type = ScalingFactor<rational::mult_t<lhs_base,rhs_base>,exp>;
};

// General case
template<long long lhs_base_num, long long lhs_base_den,
         long long lhs_exp_num, long long lhs_exp_den,
         long long rhs_base_num, long long rhs_base_den,
         long long rhs_exp_num,  long long rhs_exp_den>
struct MultT<ScalingFactor<rational::RationalConstant<lhs_base_num,lhs_base_den>,
                           rational::RationalConstant<lhs_exp_num, lhs_exp_den >
                          >,
             ScalingFactor<rational::RationalConstant<rhs_base_num,rhs_base_den>,
                           rational::RationalConstant<rhs_exp_num, rhs_exp_den >
                          >
            >
{
  using lhs = ScalingFactor<rational::RationalConstant<lhs_base_num,lhs_base_den>,
                            rational::RationalConstant<lhs_exp_num, lhs_exp_den >>;
  using rhs = ScalingFactor<rational::RationalConstant<rhs_base_num,rhs_base_den>,
                            rational::RationalConstant<rhs_exp_num, rhs_exp_den >>;

  using lhs_b = typename lhs::base;
  using lhs_e = typename lhs::exp;
  using rhs_b = typename rhs::base;
  using rhs_e = typename rhs::exp;

  using exp = rational::RationalConstant<1,lhs_e::den*rhs_e::den>;
  using base = rational::mult_t<
                  rational::pow_t<lhs_b,lhs_e::num*rhs_e::den>,
                  rational::pow_t<rhs_b,rhs_e::num*lhs_e::den>
               >;
  using type = ScalingFactor<base,exp>;
};

// template<long long lhs_base_num, long long lhs_base_den,
//          long long lhs_exp_num,  long long lhs_exp_den,
//          long long rhs_base_num, long long rhs_base_den,
//          long long rhs_exp_num,  long long rhs_exp_den>
// struct MultT<ScalingFactor<rational::RationalConstant<lhs_base_num,lhs_base_den>,
//                            rational::RationalConstant<lhs_exp_num, lhs_exp_den >
//                           >,
//              ScalingFactor<rational::RationalConstant<rhs_base_num,rhs_base_den>,
//                            rational::RationalConstant<rhs_exp_num, rhs_exp_den >
//                           >
//             >
// {

//   static constexpr bool same_base = std::is_same<lhs_b,rhs_b>::type;
//   static constexpr bool same_exp  = std::is_same<lhs_e,rhs_e>::type;

//   using type =
//     cond_t<
//       same_base,
//       ScalingFactor<lhs_b,rational::add_t<lhs_e, rhs_e>>, // same base
//       cond_t<
//         same_exp,
//         ScalingFactor<rational::mult_t<lhs_b,rhs_b>,lhs_e>, // same exp
//         ScalingFactor<
        
// };
// constexpr ScalingFactor operator* (const RationalConstant& lhs, const ScalingFactor& rhs) {
//   return rhs*ScalingFactor(lhs);
// }

// constexpr ScalingFactor operator* (const ScalingFactor& lhs, const RationalConstant& rhs) {
//   return lhs*ScalingFactor(rhs);
// }

// constexpr ScalingFactor operator/ (const ScalingFactor& lhs, const ScalingFactor& rhs) {
//   // If base or exp are the same, we can use powers properties,
//   // otherwise, recall that, with all terms being integers,
//   //    (a/b)^(c/d) / (x/y)^(w/z)
//   // is equivalent to
//   //    ((a/b)^(cz) / (x/w)^(wd)) ^ (1/dz)
//   using iType = RationalConstant::iType;

//   return lhs.base==rhs.base 
//             ? ScalingFactor(lhs.base,lhs.exp-rhs.exp)
//             : (lhs.exp==rhs.exp
//                 ? ScalingFactor(lhs.base/rhs.base,lhs.exp)
//                 : ScalingFactor( pow(lhs.base,lhs.exp.num*rhs.exp.den) / pow(rhs.base,rhs.exp.num*lhs.exp.den),
//                                  RationalConstant(iType(1),lhs.exp.den*rhs.exp.den)));
// }

// constexpr ScalingFactor operator/ (const ScalingFactor& lhs, const RationalConstant& rhs) {
//   return lhs/ScalingFactor(rhs);
// }

// constexpr ScalingFactor operator/ (const RationalConstant& lhs, const ScalingFactor& rhs) {
//   return ScalingFactor(lhs)/rhs;
// }

// constexpr ScalingFactor pow (const ScalingFactor& x, const int p) {
//   return ScalingFactor(x.base,x.exp*p);
// }

// constexpr ScalingFactor pow (const ScalingFactor& x, const RationalConstant& y) {
//   return ScalingFactor(x.base,x.exp*y);
// }

// constexpr ScalingFactor sqrt (const ScalingFactor& x) {
//   return ScalingFactor(x.base,x.exp/2);
// }



// namespace prefixes {
// constexpr ScalingFactor nano  = ScalingFactor(10,-9);
// constexpr ScalingFactor micro = ScalingFactor(10,-6);
// constexpr ScalingFactor milli = ScalingFactor(10,-3);
// constexpr ScalingFactor centi = ScalingFactor(10,-2);
// constexpr ScalingFactor deci  = ScalingFactor(10,-1);

// constexpr ScalingFactor deca  = ScalingFactor(10, 1);
// constexpr ScalingFactor hecto = ScalingFactor(10, 2);
// constexpr ScalingFactor kilo  = ScalingFactor(10, 3);
// constexpr ScalingFactor mega  = ScalingFactor(10, 6);
// constexpr ScalingFactor giga  = ScalingFactor(10, 9);
// } // namespace prefixes

template<long long base_num, long long base_den,
         long long exp_num, long long exp_den>
std::ostream& operator << (std::ostream& out, const scaling_t<base_num,base_den,exp_num,exp_den>&)
{
  out << scaling_t<base_num,base_den,exp_num,exp_den>::to_string();
  return out;
}

} // namespace scaling

} // namespace ekat

#endif // EKAT_COMPILE_TIME_SCALING_FACTOR_HPP
