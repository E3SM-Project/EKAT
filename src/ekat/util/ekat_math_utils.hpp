#ifndef EKAT_MATH_UTILS_HPP
#define EKAT_MATH_UTILS_HPP

#include "ekat/ekat.hpp"

#include <Kokkos_Core.hpp>

#ifndef KOKKOS_ENABLE_CUDA
# include <cmath>
# include <algorithm>
#endif

namespace ekat {

namespace impl {

template<typename ScalarT, bool is_integral>
struct MathFcnImpl;

template<typename ScalarT>
struct MathFcnImpl<ScalarT,true> {
  static_assert (
    std::is_integral<ScalarT>::value,
    "Error! This impl of MathFcn is meant for integral types only.\n");

#ifdef __CUDA_ARCH__
#define ekat_math_unary_fn_i(fn, cudafn)  \
  KOKKOS_FORCEINLINE_FUNCTION             \
  static double fn (ScalarT x) {          \
    return cudafn (double(x));            \
  }
#else
#define ekat_math_unary_fn_i(fn, cudafn)  \
  KOKKOS_FORCEINLINE_FUNCTION             \
  static double fn (ScalarT x) {          \
    return std::fn (x);                   \
  }
#endif

  ekat_math_unary_fn_i(abs,fabs)
  ekat_math_unary_fn_i(exp,exp)
  ekat_math_unary_fn_i(expm1,expm1)
  ekat_math_unary_fn_i(log,log)
  ekat_math_unary_fn_i(log10,log10)
  ekat_math_unary_fn_i(tgamma,tgamma)
  ekat_math_unary_fn_i(sqrt,sqrt)
  ekat_math_unary_fn_i(cbrt,cbrt)
  ekat_math_unary_fn_i(tanh,tanh)
  ekat_math_unary_fn_i(erf,erf)
};

template<typename ScalarT>
struct MathFcnImpl<ScalarT,false> {
  static_assert (
    !std::is_integral<ScalarT>::value,
    "Error! This impl of MathFcn is meant for integral types only.\n");
#ifdef __CUDA_ARCH__
#define ekat_math_unary_fn_f(fn, cudafn)  \
  KOKKOS_FORCEINLINE_FUNCTION             \
  static ScalarT fn (ScalarT x) {         \
    return cudafn (x);                    \
  }
#else
#define ekat_math_unary_fn_f(fn, cudafn)  \
  KOKKOS_FORCEINLINE_FUNCTION             \
  static ScalarT fn (ScalarT x) {         \
    return std::fn (x);                   \
  }
#endif

  ekat_math_unary_fn_f(abs,fabs)
  ekat_math_unary_fn_f(exp,exp)
  ekat_math_unary_fn_f(expm1,expm1)
  ekat_math_unary_fn_f(log,log)
  ekat_math_unary_fn_f(log10,log10)
  ekat_math_unary_fn_f(tgamma,tgamma)
  ekat_math_unary_fn_f(sqrt,sqrt)
  ekat_math_unary_fn_f(cbrt,cbrt)
  ekat_math_unary_fn_f(tanh,tanh)
  ekat_math_unary_fn_f(erf,erf)
};

} // namespace impl

template<typename ScalarT>
struct MathFcn : public impl::MathFcnImpl<ScalarT,std::is_integral<ScalarT>::value> {
  static_assert(std::is_arithmetic<ScalarT>::value,
                "Error! Scalar type for MathFcn is not a numeric type.\n");
};

namespace impl {

#ifdef KOKKOS_ENABLE_CUDA
// Replacements for namespace std functions that don't run on the GPU.
template <typename T>
KOKKOS_FORCEINLINE_FUNCTION
const T& min (const T& a, const T& b) {
  return a < b ? a : b;
}

template <typename T>
KOKKOS_FORCEINLINE_FUNCTION
const T& max (const T& a, const T& b) {
  return a > b ? a : b;
}

template <typename T>
KOKKOS_INLINE_FUNCTION
bool isfinite (const T& a) {
  return a == a && a != INFINITY && a != -INFINITY;
}

template <typename T> KOKKOS_INLINE_FUNCTION
const T* max_element (const T* const begin, const T* const end) {
  const T* me = begin;
  for (const T* it = begin + 1; it < end; ++it)
    if ( ! (*it < *me)) // use operator<
      me = it;
  return me;
}

#else
using std::min;
using std::max;
using std::isfinite;
using std::max_element;
#endif

template<typename RealT>
KOKKOS_INLINE_FUNCTION
bool is_nan (const RealT& a) {
#ifdef __CUDA_ARCH__
  return isnan(a);
#else
  return std::isnan(a);
#endif
}

template <typename Integer> KOKKOS_INLINE_FUNCTION
void set_min_max (const Integer& lim0, const Integer& lim1,
                  Integer& min, Integer& max) {
  min = impl::min(lim0, lim1);
  max = impl::max(lim0, lim1);
}

template <typename Integer, typename Integer1> KOKKOS_INLINE_FUNCTION
void set_min_max (const Integer& lim0, const Integer& lim1,
                  Integer& min, Integer& max, const Integer1& vector_size) {
  min = impl::min(lim0, lim1) / vector_size;
  max = impl::max(lim0, lim1) / vector_size;
}

template <typename Real> KOKKOS_INLINE_FUNCTION
Real rel_diff (const Real& a, const Real& b) {
  return std::abs(b - a)/std::abs(a);
}

} // namespace impl

struct TransposeDirection {
  enum Enum { c2f, f2c };
};

// Switch whether i (column index) or k (level index) is the fast
// index. TransposeDirection::c2f makes i faster; f2c makes k faster.
template <TransposeDirection::Enum direction, typename Scalar>
void transpose(const Scalar* sv, Scalar* dv, Int ni, Int nk) {
  for (Int k = 0; k < nk; ++k) {
    for (Int i = 0; i < ni; ++i) {
      const Int cidx = nk*i + k;
      const Int fidx = ni*k + i;
      if (direction == TransposeDirection::c2f) {
        dv[fidx] = sv[cidx];
      }
      else {
        dv[cidx] = sv[fidx];
      }
    }
  }
}

template <TransposeDirection::Enum direction, typename Scalar>
void transpose(const Scalar* sv, Scalar* dv, Int ni, Int nk, Int nj) {
  for (Int j = 0; j < nj; ++j) {
    for (Int k = 0; k < nk; ++k) {
      for (Int i = 0; i < ni; ++i) {
        const Int cidx = (nk*nj)*i + k*nj + j;
        const Int fidx = (ni*nk)*j + k*ni + i;
        if (direction == TransposeDirection::c2f) {
          dv[fidx] = sv[cidx];
        }
        else {
          dv[cidx] = sv[fidx];
        }
      }
    }
  }
}

} // namespace ekat

#endif // EKAT_MATH_UTILS_HPP
