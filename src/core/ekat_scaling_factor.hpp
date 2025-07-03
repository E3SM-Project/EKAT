#ifndef EKAT_SCALING_FACTOR_HPP
#define EKAT_SCALING_FACTOR_HPP

#include "ekat_rational_constant.hpp"

#include <iostream>
#include <array>

namespace ekat
{

struct ScalingFactor {
  RationalConstant base;
  RationalConstant exp;

  ScalingFactor () = delete;
  constexpr ScalingFactor (const RationalConstant& s)
   : ScalingFactor(s,1)
  {
    // Nothing to do here
  }
  constexpr ScalingFactor (const RationalConstant& b,
                           const RationalConstant& e)
   : base(std::get<0>(check_and_adjust(b,e)))
   , exp (std::get<1>(check_and_adjust(b,e)))
  {
    // Nothing to do here
  }

  constexpr ScalingFactor (const ScalingFactor&) = default;

  constexpr ScalingFactor& operator= (const ScalingFactor&) = default;

  constexpr ScalingFactor& operator*= (const ScalingFactor& rhs);

  static constexpr ScalingFactor one () { return ScalingFactor(1); }
  static constexpr ScalingFactor zero () { return ScalingFactor(0); }

  template<int N>
  constexpr std::array<char,N> string_repr () const
  {
    auto b_repr = base.string_repr<N>();
    auto e_repr = exp.string_repr<N>();

    int sz1=0, sz2=0;
    for (auto c : b_repr) {
      if (c=='\0') break;
      ++sz1;
    }
    for (auto c : e_repr) {
      if (c=='\0') break;
      ++sz2;
    }
    auto b_den_or_neg = base.den!=1 or base.num<0;
    auto e_den_or_neg = exp.den!=1 or exp.num<0;
    auto e_one = exp==RationalConstant::one();

    // If we have denominators or are negative, add parentheses, for clarity
    assert ( sz1 + (e_one ? 0 : sz2+1 + (b_den_or_neg ? 2 : 0) + (e_den_or_neg ? 2 : 0)) < N );

    std::array<char,N> out = {'\0'};
    int pos = 0;
    if (b_den_or_neg) {
      out[pos++] = '(';
    }
    for (int i=0; i<sz1; ++i) {
      out[pos++] = b_repr[i];
    }
    if (b_den_or_neg) {
      out[pos++] = ')';
    }
    if (not e_one) {
      out[pos++] = '^';
      if (e_den_or_neg) {
        out[pos++] = '(';
      }
      for (int i=0; i<sz1; ++i) {
        out[pos++] = e_repr[i];
      }
      if (e_den_or_neg) {
        out[pos++] = ')';
      }
    }
    return out;
  }

  inline std::string to_string(const Format fmt = Format::Rat) const {
    std::string s = base.to_string(fmt);
    if (exp!=RationalConstant::one()) {
      s +=  '^';
      if (exp.den!=1 and fmt==Format::Rat) {
        s+= '(';
      }
      s += exp.to_string(fmt);
      if (exp.den!=1 and fmt==Format::Rat) {
        s+= ')';
      }
    }
    return s;
  }

private:

  using base_and_exp = std::array<RationalConstant,2>;

  static constexpr base_and_exp
  check_and_adjust (const RationalConstant b,
                    const RationalConstant e)
  {
    // Check that we are not doing 0^0 or taking even roots of negative numbers.
    // If all good, adjust base and exp for x^0 case (b=1,e=1), and return what was requested.
    constexpr auto zero = RationalConstant::zero();
    constexpr auto one  = RationalConstant::one();
    if (b==zero and e==zero)
      throw std::invalid_argument("[ScalingFactor] Cannot compute 0^0");
    else if (b.num<0 && e.den%2==0)
      throw std::invalid_argument("[ScalingFactor] Cannot compute positive roots of negative numbers");

    return e==zero ? base_and_exp{{one, one}} : base_and_exp{{b,e}};
  }
};

constexpr bool operator== (const ScalingFactor& lhs, const ScalingFactor& rhs) {
  // Recall that, with all terms being integers,
  //    (a/b)^(c/d)==(x/y)^(w/z)
  // is equivalent to
  //    (a/b)^(cz) == (x/w)^(wd)
  return pow(lhs.base,lhs.exp.num*rhs.exp.den)==pow(rhs.base,rhs.exp.num*lhs.exp.den);
}

constexpr bool operator== (const ScalingFactor& lhs, const RationalConstant& rhs) {
  return lhs==ScalingFactor(rhs);
}

constexpr bool operator== (const RationalConstant& lhs, const ScalingFactor& rhs) {
  return ScalingFactor(lhs)==rhs;
}

constexpr bool operator!= (const ScalingFactor& lhs, const ScalingFactor& rhs) {
  return !(lhs==rhs);
}

constexpr ScalingFactor operator* (const ScalingFactor& lhs, const ScalingFactor& rhs) {
  // If base or exp are the same, we can use powers properties,
  // otherwise, recall that, with all terms being integers,
  //    (a/b)^(c/d) * (x/y)^(w/z)
  // is equivalent to
  //    ((a/b)^(cz) * (x/y)^(wd)) ^ (1/dz)
  using iType = RationalConstant::iType;

  return lhs.base==rhs.base 
            ? ScalingFactor(lhs.base,lhs.exp+rhs.exp)
            : (lhs.exp==rhs.exp
                ? ScalingFactor(lhs.base*rhs.base,lhs.exp)
                : ScalingFactor( pow(lhs.base,lhs.exp.num*rhs.exp.den) * pow(rhs.base,rhs.exp.num*lhs.exp.den),
                                 RationalConstant(iType(1),lhs.exp.den*rhs.exp.den)));
}

constexpr ScalingFactor operator* (const RationalConstant& lhs, const ScalingFactor& rhs) {
  return rhs*ScalingFactor(lhs);
}

constexpr ScalingFactor operator* (const ScalingFactor& lhs, const RationalConstant& rhs) {
  return lhs*ScalingFactor(rhs);
}

constexpr ScalingFactor operator/ (const ScalingFactor& lhs, const ScalingFactor& rhs) {
  // If base or exp are the same, we can use powers properties,
  // otherwise, recall that, with all terms being integers,
  //    (a/b)^(c/d) / (x/y)^(w/z)
  // is equivalent to
  //    ((a/b)^(cz) / (x/y)^(wd)) ^ (1/dz)
  using iType = RationalConstant::iType;

  return lhs.base==rhs.base 
            ? ScalingFactor(lhs.base,lhs.exp-rhs.exp)
            : (lhs.exp==rhs.exp
                ? ScalingFactor(lhs.base/rhs.base,lhs.exp)
                : ScalingFactor( pow(lhs.base,lhs.exp.num*rhs.exp.den) / pow(rhs.base,rhs.exp.num*lhs.exp.den),
                                 RationalConstant(iType(1),lhs.exp.den*rhs.exp.den)));
}

constexpr ScalingFactor operator/ (const ScalingFactor& lhs, const RationalConstant& rhs) {
  return lhs/ScalingFactor(rhs);
}

constexpr ScalingFactor operator/ (const RationalConstant& lhs, const ScalingFactor& rhs) {
  return ScalingFactor(lhs)/rhs;
}

constexpr ScalingFactor pow (const ScalingFactor& x, const int p) {
  return ScalingFactor(x.base,x.exp*p);
}

constexpr ScalingFactor pow (const ScalingFactor& x, const RationalConstant& y) {
  return ScalingFactor(x.base,x.exp*y);
}

constexpr ScalingFactor sqrt (const ScalingFactor& x) {
  return ScalingFactor(x.base,x.exp/2);
}

inline std::ostream& operator<< (std::ostream& out, const ScalingFactor& s) {
  out << s.to_string();
  return out;
}

constexpr ScalingFactor& ScalingFactor::operator*= (const ScalingFactor& rhs) {
  *this = (*this)*rhs;
  return *this;
}

namespace prefixes {
constexpr ScalingFactor nano  = ScalingFactor(10,-9);
constexpr ScalingFactor micro = ScalingFactor(10,-6);
constexpr ScalingFactor milli = ScalingFactor(10,-3);
constexpr ScalingFactor centi = ScalingFactor(10,-2);

constexpr ScalingFactor hecto = ScalingFactor(10, 2);
constexpr ScalingFactor kilo  = ScalingFactor(10, 3);
constexpr ScalingFactor mega  = ScalingFactor(10, 6);
constexpr ScalingFactor giga  = ScalingFactor(10, 9);
} // namespace prefixes

} // namespace ekat

#endif // EKAT_SCALING_FACTOR_HPP
