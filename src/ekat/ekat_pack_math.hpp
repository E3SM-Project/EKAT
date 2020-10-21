#ifndef EKAT_PACK_MATH_HPP
#define EKAT_PACK_MATH_HPP

#include "util/ekat_math_utils.hpp"

namespace ekat {

#ifdef __CUDA_ARCH__
#define ekat_pack_gen_unary_stdfn(fn)               \
  template <typename ScalarT, int N>                \
  KOKKOS_INLINE_FUNCTION                            \
  Pack<ScalarT,N> fn (const Pack<ScalarT,N>& p) {   \
    Pack<ScalarT,N> s;                              \
    vector_simd                                     \
    for (int i = 0; i < N; ++i) {                   \
      s[i] = ::fn(p[i]);                            \
    }                                               \
    return s;                                       \
  }
#else
#define ekat_pack_gen_unary_stdfn(fn)               \
  template <typename ScalarT, int N>                \
  KOKKOS_INLINE_FUNCTION                            \
  Pack<ScalarT,N> fn (const Pack<ScalarT,N>& p) {   \
    Pack<ScalarT,N> s;                              \
    vector_simd                                     \
    for (int i = 0; i < N; ++i) {                   \
      s[i] = std::fn(p[i]);                         \
    }                                               \
    return s;                                       \
  }
#endif

ekat_pack_gen_unary_stdfn(abs)
ekat_pack_gen_unary_stdfn(exp)
ekat_pack_gen_unary_stdfn(expm1)
ekat_pack_gen_unary_stdfn(log)
ekat_pack_gen_unary_stdfn(log10)
ekat_pack_gen_unary_stdfn(tgamma)
ekat_pack_gen_unary_stdfn(sqrt)
ekat_pack_gen_unary_stdfn(cbrt)
ekat_pack_gen_unary_stdfn(tanh)
ekat_pack_gen_unary_stdfn(erf)

template <typename PackType> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<PackType, typename PackType::scalar> min (const PackType& p) {
  typename PackType::scalar v(p[0]);
  vector_disabled for (int i = 0; i < PackType::n; ++i) v = impl::min(v, p[i]);
  return v;
}

template <typename PackType> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<PackType, typename PackType::scalar> max (const PackType& p) {
  typename PackType::scalar v(p[0]);
  vector_simd for (int i = 0; i < PackType::n; ++i) v = impl::max(v, p[i]);
  return v;
}

// sum = sum+p[0]+p[1]+...
// NOTE: f<bool,T> and f<T,bool> are *guaranteed* to be different overloads.
//       The latter is better when bool needs a default, the former is
//       better when bool must be specified, but we want T to be deduced.
template <bool Serialize, typename PackType>
KOKKOS_INLINE_FUNCTION
void reduce_sum (const PackType& p, typename PackType::scalar& sum) {
  if (Serialize) {
    for (int i = 0; i < PackType::n; ++i) sum += p[i];
  } else {
    vector_simd for (int i = 0; i < PackType::n; ++i) sum += p[i];
  }
}

template <typename PackType, bool Serialize = ekatBFB>
KOKKOS_INLINE_FUNCTION
void reduce_sum (const PackType& p, typename PackType::scalar& sum) {
  reduce_sum<Serialize>(p,sum);
}

// return sum = p[0]+p[1]+...
template <bool Serialize, typename PackType>
KOKKOS_INLINE_FUNCTION
OnlyPackReturn<PackType, typename PackType::scalar> reduce_sum (const PackType& p) {
  typename PackType::scalar sum = typename PackType::scalar(0);
  reduce_sum<Serialize>(p,sum);
  return sum;
}
template <typename PackType, bool Serialize = ekatBFB>
KOKKOS_INLINE_FUNCTION
OnlyPackReturn<PackType, typename PackType::scalar> reduce_sum (const PackType& p) {
  return reduce_sum<Serialize>(p);
}

// min(init, min(p(mask)))
template <typename PackType> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<PackType, typename PackType::scalar>
min (const Mask<PackType::n>& mask, typename PackType::scalar init, const PackType& p) {
  vector_disabled for (int i = 0; i < PackType::n; ++i)
    if (mask[i]) init = impl::min(init, p[i]);
  return init;
}

// max(init, max(p(mask)))
template <typename PackType> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<PackType, typename PackType::scalar>
max (const Mask<PackType::n>& mask, typename PackType::scalar init, const PackType& p) {
  vector_simd for (int i = 0; i < PackType::n; ++i)
    if (mask[i]) init = impl::max(init, p[i]);
  return init;
}

// On Intel 17 for KNL, I'm getting a ~1-ulp diff on const Scalar& b. I don't
// understand its source. But, in any case, I'm writing a separate impl here to
// get around that.
//ekat_pack_gen_bin_fn_all(pow, std::pow)
template <typename PackType, typename ScalarType>
KOKKOS_INLINE_FUNCTION
OnlyPack<PackType> pow (const PackType& a, const ScalarType/*&*/ b) {
  PackType s;
  vector_simd for (int i = 0; i < PackType::n; ++i)
    s[i] = std::pow(a[i], b);
  return s;
}

template <typename ScalarType, typename PackType>
KOKKOS_INLINE_FUNCTION
OnlyPack<PackType> pow (const ScalarType a, const PackType& b) {
  PackType s;
  vector_simd for (int i = 0; i < PackType::n; ++i)
    s[i] = std::pow(a, b[i]);
  return s;
}

template <typename PackType>
KOKKOS_INLINE_FUNCTION
OnlyPack<PackType> pow (const PackType& a, const PackType& b) {
  PackType s;
  vector_simd for (int i = 0; i < PackType::n; ++i)
    s[i] = std::pow(a[i], b[i]);
  return s;
}

template <typename PackType>
KOKKOS_INLINE_FUNCTION
OnlyPack<PackType> square (const PackType& a) {
  PackType s;
  vector_simd for (int i = 0; i < PackType::n; ++i)
    s[i] = a[i] * a[i];
  return s;
}

template <typename PackType>
KOKKOS_INLINE_FUNCTION
OnlyPack<PackType> cube (const PackType& a) {
  PackType s;
  vector_simd for (int i = 0; i < PackType::n; ++i)
    s[i] = a[i] * a[i] * a[i];
  return s;
}

} // namespace ekat

// Cleanup the macros we used simply to generate code
#undef ekat_pack_gen_unary_fn
#undef ekat_pack_gen_unary_stdfn

#endif // EKAT_PACK_MATH_HPP
