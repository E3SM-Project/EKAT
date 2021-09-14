#ifndef INCLUDE_EKAT_PACK
#define INCLUDE_EKAT_PACK

//TODO
// - bounds checking define

#include "ekat/util/ekat_math_utils.hpp"
#include "ekat/ekat_macros.hpp"
#include "ekat/ekat_scalar_traits.hpp"
#include "ekat/ekat_type_traits.hpp"

#include <Kokkos_Core.hpp>
#include "spdlog/fmt/ostr.h"
#include <iostream>
#include <type_traits>

namespace ekat {

/* API for using "packed" data in ekat. Packs are just bundles of N
   scalars within a single object. Using packed data makes it much easier
   to get good vectorization with C++.

   Pack is a vectorization pack, and Mask is a conditional mask for Pack::set
   constructed from operators among packs.

   If pack size (PACKN) is 1, then a Pack behaves as a scalar, and a Mask
   behaves roughly as a bool. Mask purposely does not support 'operator bool'
   because it is ambiguous whether operator bool should act as any() or all(),
   so we want the caller to be explicit.
 */

template <int PackSize>
struct Mask {
  // One tends to think a short boolean type would be useful here (e.g., bool or
  // char), but that is bad for vectorization. int or long are best.
  typedef long type;

  // A tag for this struct for type checking.
  enum { masktag = true };
  // Pack and Mask sizes are the same, n.
  enum { n = PackSize };

  KOKKOS_FORCEINLINE_FUNCTION
  Mask () {}

  // Init all slots of the Mask to 'init'.
  KOKKOS_FORCEINLINE_FUNCTION explicit Mask (const bool& init) {
    //vector_simd // Intel 18 is having an issue with this loop.
    vector_disabled for (int i = 0; i < n; ++i) d[i] = init;
  }

  // Set slot i to val.
  KOKKOS_FORCEINLINE_FUNCTION void set (const int& i, const bool& val) { d[i] = val; }
  // Get slot i.
  KOKKOS_FORCEINLINE_FUNCTION bool operator[] (const int& i) const { return d[i]; }

  // Is any slot true?
  KOKKOS_FORCEINLINE_FUNCTION bool any () const {
    bool b = false;
    vector_simd for (int i = 0; i < n; ++i) if (d[i]) b = true;
    return b;
  }

  // Are all slots true?
  KOKKOS_FORCEINLINE_FUNCTION bool all () const {
    bool b = true;
    vector_simd for (int i = 0; i < n; ++i) if ( ! d[i]) b = false;
    return b;
  }

  // Are all slots false?
  KOKKOS_FORCEINLINE_FUNCTION bool none () const {
    return !any();
  }

private:
  type d[n];
};

template <int n>
inline std::ostream&
operator << (std::ostream& os, const Mask<n>& m) {
  for (int i=0; i<n; ++i) {
    os << m[i] << ' ';
  }
  return os;
}

// Codify how a user can construct their own loops conditioned on mask slot
// values.
#define ekat_masked_loop(mask, s)                         \
  vector_simd for (int s = 0; s < mask.n; ++s) if (mask[s])

#define ekat_masked_loop_no_vec(mask, s)                    \
  vector_novec for (int s = 0; s < mask.n; ++s) if (mask[s])

// Implementation detail for generating binary ops for mask op mask.
#define ekat_mask_gen_bin_op_mm(op, impl)                   \
  template <int n> KOKKOS_INLINE_FUNCTION                     \
  Mask<n> operator op (const Mask<n>& a, const Mask<n>& b) {  \
    Mask<n> m;                                                \
    vector_simd for (int i = 0; i < n; ++i)                   \
      m.set(i, a[i] impl b[i]);                               \
    return m;                                                 \
  }

// Implementation detail for generating binary ops for mask op bool.
#define ekat_mask_gen_bin_op_mb(op, impl)                   \
  template <int n> KOKKOS_INLINE_FUNCTION                     \
  Mask<n> operator op (const Mask<n>& a, const bool b) {      \
    Mask<n> m;                                                \
    vector_simd for (int i = 0; i < n; ++i)                   \
      m.set(i, a[i] impl b);                                  \
    return m;                                                 \
  }

ekat_mask_gen_bin_op_mm(&&, &&)
ekat_mask_gen_bin_op_mm(||, ||)
ekat_mask_gen_bin_op_mb(&&, &&)
ekat_mask_gen_bin_op_mb(||, ||)

// Negate the mask.
template <int n> KOKKOS_INLINE_FUNCTION
Mask<n> operator ! (const Mask<n>& m) {
  Mask<n> not_m;
  vector_simd for (int i = 0; i < n; ++i) not_m.set(i, ! m[i]);
  return not_m;
}

// Implementation detail for generating Pack assignment operators. _p means the
// input is a Pack; _s means the input is a scalar.
// NOTE: for volatile overload, you should return void. If not, if/when Kokkos
//       atomic ops are compiled for Pack, you will see a compiler warning like
//   warning: implicit dereference will not access object of type ‘volatile ekat::Pack<...>' in statement
//       *dest = return_val + val;
//       ^
#define ekat_pack_gen_assign_op_p(op)                       \
  KOKKOS_FORCEINLINE_FUNCTION                               \
  Pack& operator op (const volatile Pack& a) {              \
    vector_simd for (int i = 0; i < n; ++i) d[i] op a.d[i]; \
    return *this;                                           \
  }                                                         \
  KOKKOS_FORCEINLINE_FUNCTION                               \
  Pack& operator op (const Pack& a) {                       \
    vector_simd for (int i = 0; i < n; ++i) d[i] op a[i];   \
    return *this;                                           \
  }                                                         \
  KOKKOS_FORCEINLINE_FUNCTION                               \
  void operator op (const Pack& a) volatile {               \
    vector_simd for (int i = 0; i < n; ++i) d[i] op a.d[i]; \
  }                                                         \
  KOKKOS_FORCEINLINE_FUNCTION                               \
  void operator op (const volatile Pack& a) volatile {      \
    vector_simd for (int i = 0; i < n; ++i) d[i] op a.d[i]; \
  }
#define ekat_pack_gen_assign_op_s(op)                       \
  KOKKOS_FORCEINLINE_FUNCTION                               \
  Pack& operator op (const scalar& a) {                     \
    vector_simd for (int i = 0; i < n; ++i) d[i] op a;      \
    return *this;                                           \
  }
#define ekat_pack_gen_assign_op_all(op)       \
  ekat_pack_gen_assign_op_p(op)               \
  ekat_pack_gen_assign_op_s(op)

// The Pack type. Mask was defined first since it's used in Pack.
template <typename ScalarType, int PackSize>
struct Pack {
  // Pack's tag for type checking.
  enum { packtag = true };
  // Number of slots in the Pack.
  enum { n = PackSize };

  // A definitely non-const version of the input scalar type.
  typedef typename std::remove_const<ScalarType>::type scalar;

  KOKKOS_FORCEINLINE_FUNCTION
  Pack () {
    vector_simd for (int i = 0; i < n; ++i) {
      d[i] = ScalarTraits<scalar>::invalid();
    }
  }

  // Init all slots to scalar v.
  KOKKOS_FORCEINLINE_FUNCTION
  Pack (const scalar& v) {
    vector_simd for (int i = 0; i < n; ++i) d[i] = v;
  }

  // Init this Pack from another one.
  template <typename T>
  KOKKOS_FORCEINLINE_FUNCTION explicit
  Pack (const Pack<T,n>& v) {
    vector_simd for (int i = 0; i < n; ++i) d[i] = v[i];
  }

  // Init this Pack from another one.
  KOKKOS_FORCEINLINE_FUNCTION
  Pack (const Pack& src) {
    vector_simd for (int i = 0; i < n; ++i) d[i] = src[i];
  }

  // Init this Pack from another one.
  KOKKOS_FORCEINLINE_FUNCTION
  Pack (const volatile Pack& src) {
    vector_simd for (int i = 0; i < n; ++i) d[i] = src.d[i];
  }

  // Init this Pack from a scalar, but only where Mask is true; otherwise
  // init to default value.
  template <typename T>
  KOKKOS_FORCEINLINE_FUNCTION
  explicit Pack (const Mask<n>& m, const T p) {
    vector_simd for (int i = 0; i < n; ++i) {
      d[i] = m[i] ? p : ScalarTraits<scalar>::invalid();
    }
  }

  // Init this Pack from two scalars, according to a given mask:
  // if mask is true, set first value, otherwise the other.
  template <typename T, typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  explicit Pack (const Mask<n>& m, const T v_true, const S v_false) {
    vector_simd
    for (int i = 0; i < n; ++i) {
      d[i] = m[i] ? v_true : v_false;
    }
  }

  // Init this Pack from a scalar, but only where Mask is true; otherwise
  // init to default value.
  template <typename T>
  KOKKOS_FORCEINLINE_FUNCTION
  explicit Pack (const Mask<n>& m, const Pack<T,n>& p) {
    vector_simd for (int i = 0; i < n; ++i) {
      d[i] = m[i] ? p[i] : ScalarTraits<scalar>::invalid();
    }
  }

  // Init this Pack from two other packs, according to a given mask:
  // if mask is true, set first pack's value, otherwise the other's.
  template <typename T, typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  explicit Pack (const Mask<n>& m, const Pack<T,n>& p_true, const Pack<S,n>& p_false) {
    vector_simd
    for (int i = 0; i < n; ++i) {
      d[i] = m[i] ? p_true[i] : p_false[i];
    }
  }

  KOKKOS_FORCEINLINE_FUNCTION const scalar& operator[] (const int& i) const { return d[i]; }
  KOKKOS_FORCEINLINE_FUNCTION scalar& operator[] (const int& i) { return d[i]; }

  ekat_pack_gen_assign_op_all(=)
  ekat_pack_gen_assign_op_all(+=)
  ekat_pack_gen_assign_op_all(-=)
  ekat_pack_gen_assign_op_all(*=)
  ekat_pack_gen_assign_op_all(/=)

  KOKKOS_FORCEINLINE_FUNCTION
  Pack& set (const Mask<n>& mask, const scalar& v) {
    vector_simd for (int i = 0; i < n; ++i) if (mask[i]) d[i] = v;

    return *this;
  }

  template <typename PackIn>
  KOKKOS_FORCEINLINE_FUNCTION
  Pack& set (const Mask<n>& mask, const PackIn& p,
            typename std::enable_if<PackIn::packtag>::type* = nullptr) {
    static_assert(static_cast<int>(PackIn::n) == PackSize,
                  "Pack::n must be the same.");
    vector_simd for (int i = 0; i < n; ++i) if (mask[i]) d[i] = p[i];

    return *this;
  }

  KOKKOS_FORCEINLINE_FUNCTION
  Pack& set (const Mask<n>& mask, const scalar& v_true, const scalar& v_false) {
    vector_simd
    for (int i = 0; i < n; ++i) {
      if (mask[i])
        d[i] = v_true;
      else
        d[i] = v_false;
    }

    return *this;
  }

  template <typename T, typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  Pack& set (const Mask<n>& mask, const Pack<T,n>& p_true, const Pack<S,n>& p_false) {
    vector_simd
    for (int i = 0; i < n; ++i) {
      if (mask[i])
        d[i] = p_true[i];
      else
        d[i] = p_false[i];
    }

    return *this;
  }

private:
  scalar d[n];
};

// Use enable_if and packtag so that we can template on 'Pack' and yet not have
// our operator overloads, in particular, be used for something other than the
// Pack type.
template <typename PackType>
using OnlyPack = typename std::enable_if<PackType::packtag,PackType>::type;
template <typename PackType, typename ReturnType>
using OnlyPackReturn = typename std::enable_if<PackType::packtag,ReturnType>::type;

// Later, we might support type promotion. For now, caller must explicitly
// promote a pack's scalar type in mixed-type arithmetic.

#define ekat_pack_gen_bin_op_pp(op)                                   \
  template <typename T, int n>                                          \
  KOKKOS_FORCEINLINE_FUNCTION                                           \
  Pack<T,n>                                                             \
  operator op (const Pack<T,n>& a, const Pack<T,n>& b) {                \
    Pack<T,n> c;                                                        \
    vector_simd                                                         \
    for (int i = 0; i < n; ++i) c[i] = a[i] op b[i];                    \
    return c;                                                           \
  }
#define ekat_pack_gen_bin_op_ps(op)                                   \
  template <typename T, int n, typename ScalarType>                     \
  KOKKOS_FORCEINLINE_FUNCTION                                           \
  Pack<T,n>                                                             \
  operator op (const Pack<T,n>& a, const ScalarType& b) {               \
    Pack<T,n> c;                                                        \
    vector_simd                                                         \
    for (int i = 0; i < n; ++i) c[i] = a[i] op b;                       \
    return c;                                                           \
  }
#define ekat_pack_gen_bin_op_sp(op)                                   \
  template <typename T, int n, typename ScalarType>                     \
  KOKKOS_FORCEINLINE_FUNCTION                                           \
  Pack<T,n>                                                             \
  operator op (const ScalarType& a, const Pack<T,n>& b) {               \
    Pack<T,n> c;                                                        \
    vector_simd                                                         \
    for (int i = 0; i < n; ++i) c[i] = a op b[i];                       \
    return c;                                                           \
  }
#define ekat_pack_gen_bin_op_all(op)          \
  ekat_pack_gen_bin_op_pp(op)                 \
  ekat_pack_gen_bin_op_ps(op)                 \
  ekat_pack_gen_bin_op_sp(op)

ekat_pack_gen_bin_op_all(+)
ekat_pack_gen_bin_op_all(-)
ekat_pack_gen_bin_op_all(*)
ekat_pack_gen_bin_op_all(/)

#define ekat_pack_gen_unary_op(op)                \
  template <typename T, int n>                    \
  KOKKOS_FORCEINLINE_FUNCTION                     \
  Pack<T,n>                                       \
  operator op (const Pack<T,n>& a) {              \
    Pack<T,n> b;                                  \
    vector_simd                                   \
    for (int i = 0; i < n; ++i) b[i] = op a[i];   \
    return b;                                     \
  }

ekat_pack_gen_unary_op(-)

#define ekat_pack_gen_bin_fn_pp(fn, impl)                   \
  template <typename T, int n> KOKKOS_INLINE_FUNCTION       \
  Pack<T,n> fn (const Pack<T,n>& a, const Pack<T,n>& b) {   \
    Pack<T,n> s;                                            \
    vector_simd for (int i = 0; i < n; ++i)                 \
      s[i] = impl(a[i], b[i]);                              \
    return s;                                               \
  }
#define ekat_pack_gen_bin_fn_ps(fn, impl)                 \
  template <typename T, int n, typename ScalarType>       \
  KOKKOS_INLINE_FUNCTION                                  \
  Pack<T,n>                                               \
  fn (const Pack<T,n>& a, const ScalarType& b) {          \
    Pack<T,n> s;                                          \
    vector_simd for (int i = 0; i < n; ++i)               \
      s[i] = impl<typename Pack<T,n>::scalar>(a[i], b);   \
    return s;                                             \
  }
#define ekat_pack_gen_bin_fn_sp(fn, impl)                   \
  template <typename T, int n, typename ScalarType>         \
  KOKKOS_INLINE_FUNCTION                                    \
  Pack<T,n> fn (const ScalarType& a, const Pack<T,n>& b) {  \
    Pack<T,n> s;                                            \
    vector_simd for (int i = 0; i < n; ++i)                 \
      s[i] = impl<typename Pack<T,n>::scalar>(a, b[i]);     \
    return s;                                               \
  }
#define ekat_pack_gen_bin_fn_all(fn, impl)    \
  ekat_pack_gen_bin_fn_pp(fn, impl)           \
  ekat_pack_gen_bin_fn_ps(fn, impl)           \
  ekat_pack_gen_bin_fn_sp(fn, impl)

ekat_pack_gen_bin_fn_all(min, impl::min)
ekat_pack_gen_bin_fn_all(max, impl::max)

template <typename T, int n>
KOKKOS_INLINE_FUNCTION
Pack<T,n> shift_right (const Pack<T,n>& pm1, const Pack<T,n>& p) {
  Pack<T,n> s;
  s[0] = pm1[n-1];
  vector_simd for (int i = 1; i < n; ++i) s[i] = p[i-1];
  return s;
}

template <typename T, int n, typename ScalarType>
KOKKOS_INLINE_FUNCTION
Pack<T,n> shift_right (const ScalarType& pm1, const Pack<T,n>& p) {
  Pack<T,n> s;
  s[0] = pm1;
  vector_simd for (int i = 1; i < n; ++i) s[i] = p[i-1];
  return s;
}

template <typename T, int n>
KOKKOS_INLINE_FUNCTION
Pack<T,n> shift_left (const Pack<T,n>& pp1, const Pack<T,n>& p) {
  Pack<T,n> s;
  s[n-1] = pp1[0];
  vector_simd for (int i = 0; i < n-1; ++i) s[i] = p[i+1];
  return s;
}

template <typename T, int n, typename ScalarType>
KOKKOS_INLINE_FUNCTION
Pack<T,n> shift_left (const ScalarType& pp1, const Pack<T,n>& p) {
  Pack<T,n> s;
  s[n-1] = pp1;
  vector_simd for (int i = 0; i < n-1; ++i) s[i] = p[i+1];
  return s;
}

#define ekat_mask_gen_bin_op_pp(op)                       \
  template <typename T, int n>                            \
  KOKKOS_INLINE_FUNCTION                                  \
  Mask<n>                                                 \
  operator op (const Pack<T,n>& a, const Pack<T,n>& b) {  \
    Mask<n> m;                                            \
    vector_simd for (int i = 0; i < n; ++i)               \
      m.set(i, a[i] op b[i]);                             \
    return m;                                             \
  }
#define ekat_mask_gen_bin_op_ps(op)                         \
  template <typename T, int n, typename ScalarType>         \
  KOKKOS_INLINE_FUNCTION                                    \
  Mask<n>                                                   \
  operator op (const Pack<T,n>& a, const ScalarType& b) {   \
    Mask<n> m;                                              \
    vector_simd for (int i = 0; i < n; ++i)                 \
      m.set(i, a[i] op b);                                  \
    return m;                                               \
  }
#define ekat_mask_gen_bin_op_sp(op)                         \
  template <typename T, int n, typename ScalarType>         \
  KOKKOS_INLINE_FUNCTION                                    \
  Mask<n>                                                   \
  operator op (const ScalarType& a, const Pack<T,n>& b) {   \
    Mask<n> m;                                              \
    vector_simd for (int i = 0; i < n; ++i)                 \
      m.set(i, a op b[i]);                                  \
    return m;                                               \
  }
#define ekat_mask_gen_bin_op_all(op)          \
  ekat_mask_gen_bin_op_pp(op)                 \
  ekat_mask_gen_bin_op_ps(op)                 \
  ekat_mask_gen_bin_op_sp(op)

ekat_mask_gen_bin_op_all(==)
ekat_mask_gen_bin_op_all(!=)
ekat_mask_gen_bin_op_all(>=)
ekat_mask_gen_bin_op_all(<=)
ekat_mask_gen_bin_op_all(>)
ekat_mask_gen_bin_op_all(<)

template <typename T, int n>
KOKKOS_INLINE_FUNCTION
Mask<n>
isnan (const Pack<T,n>& p) {
  Mask<n> m;
  vector_simd for (int i = 0; i < n; ++i) {
    m.set(i, impl::is_nan(p[i]));
  }
  return m;
}


template <typename PackType>
KOKKOS_INLINE_FUNCTION
OnlyPackReturn<PackType,Int> npack(const Int& nscalar) {
  return (nscalar + PackType::n - 1) / PackType::n;
}

template <typename PackType>
KOKKOS_INLINE_FUNCTION
OnlyPack<PackType> range (const typename PackType::scalar& start) {
  PackType p;
  vector_simd for (int i = 0; i < PackType::n; ++i) p[i] = start + i;
  return p;
}
// Specialization of ScalarTraits struct for Pack types

template<typename T, int N>
struct ScalarTraits<Pack<T,N>> {

  using inner_traits = ScalarTraits<T>;

  using value_type  = Pack<T,N>;
  using scalar_type = typename inner_traits::scalar_type;

  // TODO: should we allow a pack of packs? For now, I assume the answer is NO.
  //       So in order to FORBID pack<pack<T,N>>, check that inner_traits::value_type
  //       is an arithmetic type.
  static_assert (std::is_arithmetic<typename inner_traits::value_type>::value,
                 "Error! We do not allow nested pack structures, for now.\n");

  // This seems funky. But write down a pow of 2 and a non-pow of 2 in binary (both positive), and you'll see why it works
  static_assert (N>0 && ((N & (N-1))==0), "Error! We only support packs with length = 2^n.\n");

  // Roll our own (hopefully verbose enough) name for this type.
  static std::string name () {
    return "Pack<" + inner_traits::name() + "," + std::to_string(N) + ">";
  }
  static constexpr bool is_simd = true;

  KOKKOS_INLINE_FUNCTION
  static const value_type quiet_NaN () {
    return value_type(inner_traits::quiet_NaN());
  }

  KOKKOS_INLINE_FUNCTION
  static const value_type invalid () {
    return value_type(inner_traits::invalid());
  }
};

template <typename PackType>
inline typename std::enable_if<PackType::packtag, std::ostream&>::type
operator << (std::ostream& os, const PackType& p) {
  for (int i=0; i<PackType::n; ++i) {
    os << p[i] << ' ';
  }
  return os;
}


} // namespace ekat

#include "ekat_pack_math.hpp"

// Cleanup the macros we used simply to generate code
#undef ekat_pack_gen_assign_op_p
#undef ekat_pack_gen_assign_op_s
#undef ekat_pack_gen_assign_op_all
#undef ekat_pack_gen_bin_op_pp
#undef ekat_pack_gen_bin_op_ps
#undef ekat_pack_gen_bin_op_sp
#undef ekat_pack_gen_bin_op_all
#undef ekat_pack_gen_unary_op
#undef ekat_pack_gen_bin_fn_pp
#undef ekat_pack_gen_bin_fn_ps
#undef ekat_pack_gen_bin_fn_sp
#undef ekat_pack_gen_bin_fn_all
#undef ekat_mask_gen_bin_op_pp
#undef ekat_mask_gen_bin_op_ps
#undef ekat_mask_gen_bin_op_sp
#undef ekat_mask_gen_bin_op_all
#undef ekat_mask_gen_bin_op_mm
#undef ekat_mask_gen_bin_op_mb

#endif // INCLUDE_EKAT_PACK
