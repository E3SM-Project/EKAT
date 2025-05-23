#ifndef EKAT_MATH_UTILS_HPP
#define EKAT_MATH_UTILS_HPP

#include <ekat_scalar_traits.hpp>

#include <Kokkos_Core.hpp>

#ifndef EKAT_ENABLE_GPU
# include <cmath>
# include <algorithm>
#endif

namespace ekat {

namespace impl {

#ifdef EKAT_ENABLE_GPU
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
using std::max_element;
#endif

template <typename T>
KOKKOS_INLINE_FUNCTION
bool isfinite (const T& a) {
  return Kokkos::isfinite(a);
}

template<typename RealT>
KOKKOS_INLINE_FUNCTION
bool is_nan (const RealT& a) {
  return Kokkos::isnan(a);
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

template<typename T>
KOKKOS_INLINE_FUNCTION
constexpr std::enable_if_t<!ScalarTraits<T>::specialized,T>
invalid () {
  if constexpr (ScalarTraits<T>::is_floating_point) {
    return Kokkos::Experimental::quiet_NaN_v<T>;
  } else {
    return Kokkos::Experimental::finite_max_v<T>;
  }
}

struct TransposeDirection {
  enum Enum { c2f, f2c };
};

// Switch whether i (column index) or k (level index) is the fast
// index. TransposeDirection::c2f makes i faster; f2c makes k faster.
template <TransposeDirection::Enum direction, typename Scalar>
void transpose(const Scalar* sv, Scalar* dv, int ni, int nk) {
  for (int k = 0; k < nk; ++k) {
    for (int i = 0; i < ni; ++i) {
      const int cidx = nk*i + k;
      const int fidx = ni*k + i;
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
void transpose(const Scalar* sv, Scalar* dv, int ni, int nk, int nj) {
  for (int j = 0; j < nj; ++j) {
    for (int k = 0; k < nk; ++k) {
      for (int i = 0; i < ni; ++i) {
        const int cidx = (nk*nj)*i + k*nj + j;
        const int fidx = (ni*nk)*j + k*ni + i;
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
