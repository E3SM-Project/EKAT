#ifndef EKAT_PACK_WHERE_HPP
#define EKAT_PACK_WHERE_HPP

#include "ekat_where.hpp"
#include "ekat_pack.hpp"
#include "ekat_pack_math.hpp"

namespace ekat
{

// Partial specialization of where_expression for Pack value types.
// MaskT may be Mask<N> (stored by value) or Mask<N>& (stored by reference),
// enabling perfect forwarding: pass a mask lvalue to avoid copying the mask.
template<typename MaskT, typename V, int N>
class where_expression<MaskT, Pack<V,N>>
{
public:
  using mask_t   = Mask<N>;
  using value_t  = Pack<V,N>;
  using scalar_t = typename ekat::ScalarTraits<value_t>::scalar_type;

  static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<MaskT>>, Mask<N>>,
      "Error! Pack where_expression requires a Mask<N> mask type (possibly a ref and/or cv-qualified).\n");

  // MaskT may be a reference type (e.g., bool&), in which case the mask is
  // stored by reference, so changes to the original mask are reflected here.
  // But it may be an rvalue, in which case we copy it, to avoid dangling refs
  using stored_mask_t = std::conditional_t<
    std::is_lvalue_reference_v<MaskT>,
    const mask_t&,
    mask_t
  >;

  KOKKOS_FORCEINLINE_FUNCTION
  where_expression (MaskT&& m, value_t& v)
   : m_mask (std::forward<MaskT>(m)) , m_value (v)
  { /* Nothing to do here */ }

  KOKKOS_FORCEINLINE_FUNCTION
        value_t& value ()       { return m_value; }
  KOKKOS_FORCEINLINE_FUNCTION
  const value_t& value () const { return m_value; }

  KOKKOS_FORCEINLINE_FUNCTION
  const mask_t& mask () const { return m_mask; }

  // Generate scalar and pack overloads of compound assignment operators.
#define where_pack_update_op_s(op)                                               \
  template<typename S>                                                           \
  KOKKOS_FORCEINLINE_FUNCTION                                                    \
  typename std::enable_if<std::is_convertible<S,V>::value, value_t&>::type       \
  operator op(const S& rhs) {                                                    \
    ekat_masked_loop(m_mask, i) m_value[i] op rhs;                               \
    return m_value;                                                              \
  }

#define where_pack_update_op_p(op)                                               \
  template<typename S>                                                           \
  KOKKOS_FORCEINLINE_FUNCTION                                                    \
  typename std::enable_if<std::is_convertible<S,V>::value, value_t&>::type       \
  operator op(const Pack<S,N>& rhs) {                                            \
    ekat_masked_loop(m_mask, i) m_value[i] op rhs[i];                            \
    return m_value;                                                              \
  }

#define where_pack_update_op(op) \
  where_pack_update_op_s(op)     \
  where_pack_update_op_p(op)

  where_pack_update_op(=)
  where_pack_update_op(+=)
  where_pack_update_op(-=)
  where_pack_update_op(*=)
  where_pack_update_op(/=)

#undef where_pack_update_op_s
#undef where_pack_update_op_p
#undef where_pack_update_op

  // Reduce over valid (masked) entries; return identity for unmasked positions.
  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t reduce_max () const {
    auto ret = Kokkos::reduction_identity<scalar_t>::max();
    ekat_masked_loop(m_mask, i)
      ret = impl::max(ret, m_value[i]);
    return ret;
  }
  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t reduce_min () const {
    auto ret = Kokkos::reduction_identity<scalar_t>::min();
    ekat_masked_loop(m_mask, i)
      ret = impl::min(ret, m_value[i]);
    return ret;
  }
  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t reduce_sum () const {
    auto ret = Kokkos::reduction_identity<scalar_t>::sum();
    ekat_masked_loop(m_mask, i)
      ret += m_value[i];
    return ret;
  }

  KOKKOS_FORCEINLINE_FUNCTION
  bool any  () const { return m_mask.any(); }
  KOKKOS_FORCEINLINE_FUNCTION
  bool all  () const { return m_mask.all(); }
  KOKKOS_FORCEINLINE_FUNCTION
  bool none () const { return m_mask.none(); }
private:
  stored_mask_t m_mask;
  value_t&      m_value;
};

// ----- Create where expressions from mask and pack objects ------ //

template<typename MaskT, typename S, int N>
KOKKOS_FORCEINLINE_FUNCTION
auto where (MaskT&& mask, Pack<S,N>& p)
{
  return where_expression<MaskT, Pack<S,N>>(std::forward<MaskT>(mask), p);
}

// Binary math functions: apply fcn element-wise only where mask is true.
// Note: to avoid compute fcns of bad inputs, we do the follow this pattern:
//   0. set result=input;
//   1. sanitize: set result=1 where mask=false
//   2. call math fcn for the whole pack
//   3. set output to invalid where mask=false
// 0 and 1 can be done in one sweep
#define ekat_gen_where_binary_fcn(fcn,unset_in,unset_out)                       \
template<typename MaskT, typename V, int N, typename RhsT>                      \
KOKKOS_FORCEINLINE_FUNCTION                                                     \
Pack<V,N> fcn (const where_expression<MaskT, Pack<V,N>>& lhs, const RhsT& rhs)  \
{                                                                               \
  Pack<V,N> tmp(lhs.mask(),lhs.value(),unset_in);                               \
  auto result = fcn(tmp,rhs);                                                   \
  result.set(lhs.mask(),result,unset_out);                                      \
  return result;                                                                \
}                                                                               \
template<typename LhsT, typename MaskT, typename V, int N>                      \
KOKKOS_FORCEINLINE_FUNCTION                                                     \
Pack<V,N> fcn (const LhsT& lhs, const where_expression<MaskT, Pack<V,N>>& rhs)  \
{                                                                               \
  Pack<V,N> tmp(rhs.mask(),rhs.value(),unset_in);                               \
  auto result = fcn(lhs,tmp);                                                   \
  result.set(rhs.mask(),result,unset_out);                                      \
  return result;                                                                \
}                                                                               \
template<typename MaskT, typename V, int N>                                     \
KOKKOS_FORCEINLINE_FUNCTION                                                     \
Pack<V,N> fcn (const where_expression<MaskT, Pack<V,N>>& lhs,                   \
               const where_expression<MaskT, Pack<V,N>>& rhs)                   \
{                                                                               \
  Pack<V,N> tmp1(lhs.mask(),lhs.value(),unset_in);                              \
  Pack<V,N> tmp2(rhs.mask(),rhs.value(),unset_in);                              \
  auto result = fcn(tmp1,tmp2);                                                 \
  result.set(lhs.mask() && rhs.mask(),result,unset_out);                        \
  return result;                                                                \
}

ekat_gen_where_binary_fcn(pow,1,invalid<V>())
ekat_gen_where_binary_fcn(max,Kokkos::reduction_identity<V>::max(),Kokkos::reduction_identity<V>::max())
ekat_gen_where_binary_fcn(min,Kokkos::reduction_identity<V>::min(),Kokkos::reduction_identity<V>::min())
#undef ekat_gen_where_binary_fcn

// Unary math functions: apply fcn element-wise only where mask is true.
// #define ekat_gen_where_unary_fcn(fcn,unset_in)                \
// template<typename MaskT, typename V, int N>                   \
// KOKKOS_FORCEINLINE_FUNCTION                                   \
// Pack<V,N> fcn (const where_expression<MaskT, Pack<V,N>>& w)   \
// {                                                             \
  // ekat_masked_loop(w.mask(),i)                                \
      // res[i] = Kokkos::fcn(w.value()[i]);                     \
    // else                                                      \
      // res[i] = invalid<V>();                                  \
// }
#define ekat_gen_where_unary_fcn(fcn,unset_in)                \
template<typename MaskT, typename V, int N>                   \
KOKKOS_FORCEINLINE_FUNCTION                                   \
Pack<V,N> fcn (const where_expression<MaskT, Pack<V,N>>& w)   \
{                                                             \
  if (w.mask().none()) return invalid<V>();                   \
  if (w.mask().all()) return fcn(w.value());                  \
  Pack<V,N> tmp(w.mask(),w.value(),unset_in);                 \
  auto result = fcn(tmp);                                     \
  result.set(w.mask(),result,invalid<V>());                   \
  return result;                                              \
}

ekat_gen_where_unary_fcn(abs,1)
ekat_gen_where_unary_fcn(sin,1)
ekat_gen_where_unary_fcn(cos,1)
ekat_gen_where_unary_fcn(exp,1)
ekat_gen_where_unary_fcn(expm1,1)
ekat_gen_where_unary_fcn(log,1)
ekat_gen_where_unary_fcn(log10,1)
ekat_gen_where_unary_fcn(tgamma,1)
ekat_gen_where_unary_fcn(sqrt,1)
ekat_gen_where_unary_fcn(cbrt,1)
ekat_gen_where_unary_fcn(tanh,1)
ekat_gen_where_unary_fcn(erf,1)

#undef ekat_gen_where_unary_fcn

} // namespace ekat

#endif // EKAT_PACK_WHERE_HPP
