#ifndef EKAT_MATH_UTILS_HPP
#define EKAT_MATH_UTILS_HPP

#include "ekat/ekat_scalar_traits.hpp"
#include "ekat/ekat.hpp"

#include <Kokkos_Core.hpp>

#ifndef KOKKOS_ENABLE_CUDA
# include <cmath>
# include <algorithm>
#endif

namespace ekat {

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

template<typename ScalarT>
KOKKOS_INLINE_FUNCTION
typename std::enable_if<std::is_floating_point<ScalarT>::value,bool>::type
is_invalid (const ScalarT& a) {
  // Note: we can't do 'a==ScalarTraits<RealT>::invalid()',
  //       since for floating point, invalid=nan, and nan
  //       does not evaluate equal to anything, including itself.
  return impl::is_nan(a);
}
template<typename ScalarT>
KOKKOS_INLINE_FUNCTION
typename std::enable_if<!std::is_floating_point<ScalarT>::value,bool>::type
is_invalid (const ScalarT& a) {
  return a==ScalarTraits<ScalarT>::invalid();
}

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
