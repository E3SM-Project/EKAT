#ifndef EKAT_WHERE_HPP
#define EKAT_WHERE_HPP

#include "ekat_scalar_traits.hpp"
#include "ekat_math_utils.hpp"

#include <Kokkos_Core.hpp>

namespace ekat
{

// A where expression allows to store a handle to a scalar,
// along with a mask. When performing arithmetic operations,
// we only perform them wherever the mask is 'true'.
// For the base implementation, we REQUIRE that the scalar
// is NOT a simd-like type.

template<typename MaskT, typename V>
class where_expression
{
public:
  using mask_t   = std::remove_cv_t<std::remove_reference_t<MaskT>>;
  using value_t  = V;
  using scalar_traits = ekat::ScalarTraits<value_t>;
  using scalar_t = typename scalar_traits::scalar_type;

  static_assert (not scalar_traits::is_simd,
      "Error! Base impl of where_expression only works for non-simd types.\n");

  // MaskT may be a reference type (e.g., bool&), in which case the mask is
  // stored by reference, so changes to the original mask are reflected here.
  // But it may be an rvalue, in which case we copy it, to avoid dangling refs
  using stored_mask_t = std::conditional_t<
    std::is_lvalue_reference_v<MaskT>,
    const mask_t&,
    mask_t
  >;

  KOKKOS_FORCEINLINE_FUNCTION
  where_expression (MaskT&& m, V& v)
   : m_mask (std::forward<MaskT>(m)) , m_value (v)
  { /* Nothing to do here */ }

  KOKKOS_FORCEINLINE_FUNCTION
        value_t& value ()       { return m_value; }
  KOKKOS_FORCEINLINE_FUNCTION
  const value_t& value () const { return m_value; }

  KOKKOS_FORCEINLINE_FUNCTION
  const mask_t& mask () const { return m_mask; }

  // Reduce over valid (masked) entries; return identity when mask is false.
  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t reduce_max () const {
    if (m_mask)
      return m_value;
    return Kokkos::reduction_identity<scalar_t>::max();
  }
  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t reduce_min () const {
    if (m_mask)
      return m_value;
    return Kokkos::reduction_identity<scalar_t>::min();
  }
  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t reduce_sum () const {
    if (m_mask)
      return m_value;
    return Kokkos::reduction_identity<scalar_t>::sum();
  }

  // Generate assignment and compound assignment ops
#define where_update_op(op)               \
  template<typename S>                    \
  KOKKOS_FORCEINLINE_FUNCTION             \
  value_t& operator op(const S& rhs)  {   \
    if (m_mask) m_value op rhs;           \
    return m_value;                       \
  }

  where_update_op(=)
  where_update_op(+=)
  where_update_op(-=)
  where_update_op(*=)
  where_update_op(/=)
#undef where_update_op

  KOKKOS_FORCEINLINE_FUNCTION
  bool any  () const { return m_mask; }
  KOKKOS_FORCEINLINE_FUNCTION
  bool all  () const { return m_mask; }
  KOKKOS_FORCEINLINE_FUNCTION
  bool none () const { return !m_mask; }
private:
  stored_mask_t m_mask;
  value_t&      m_value;
};

// ----- Create where expressions from mask-like and value objects ------ //

template<typename MaskT, typename S>
KOKKOS_FORCEINLINE_FUNCTION
auto where (MaskT&& mask, S& scalar)
{
  return where_expression<MaskT,S>(std::forward<MaskT>(mask),scalar);
}

// Binary math functions: apply fcn(lhs, rhs) only where the relevant mask(s) are true.
// NOTE: if the mask is false, return the value stored in the where_expression
#define ekat_gen_where_binary_fcn(fcn,unset_out)                    \
template<typename MaskT, typename V, typename RhsT>                 \
KOKKOS_FORCEINLINE_FUNCTION                                         \
auto fcn (const where_expression<MaskT,V>& lhs, const RhsT& rhs)    \
{                                                                   \
  return lhs.mask() ? Kokkos::fcn(lhs.value(), rhs)                 \
                    : unset_out;                                    \
}                                                                   \
template<typename LhsT, typename MaskT, typename V>                 \
KOKKOS_FORCEINLINE_FUNCTION                                         \
auto fcn (const LhsT& lhs, const where_expression<MaskT,V>& rhs)    \
{                                                                   \
  return rhs.mask() ? Kokkos::fcn(lhs, rhs.value())                 \
                    : unset_out;                                    \
}                                                                   \
template<typename MaskT, typename V>                                \
KOKKOS_FORCEINLINE_FUNCTION                                         \
auto fcn (const where_expression<MaskT,V>& lhs,                     \
          const where_expression<MaskT,V>& rhs)                     \
{                                                                   \
  return lhs.mask() && rhs.mask()                                   \
      ? Kokkos::fcn(lhs.value(), rhs.value())                       \
      : unset_out;                                                  \
}

ekat_gen_where_binary_fcn(pow,invalid<V>())
ekat_gen_where_binary_fcn(min,Kokkos::reduction_identity<V>::min())
ekat_gen_where_binary_fcn(max,Kokkos::reduction_identity<V>::max())
#undef ekat_gen_where_binary_fcn

// Unary math functions: apply fcn(x) only where mask is true.
#define ekat_gen_where_unary_fcn(fcn,unset_out)                   \
template<typename MaskT,typename V>                               \
KOKKOS_FORCEINLINE_FUNCTION                                       \
auto fcn (const where_expression<MaskT,V>& w)                     \
{                                                                 \
  return w.mask() ? Kokkos::fcn(w.value()) : unset_out;           \
}

ekat_gen_where_unary_fcn(abs,invalid<V>())
ekat_gen_where_unary_fcn(sin,invalid<V>())
ekat_gen_where_unary_fcn(cos,invalid<V>())
ekat_gen_where_unary_fcn(exp,invalid<V>())
ekat_gen_where_unary_fcn(expm1,invalid<V>())
ekat_gen_where_unary_fcn(log,invalid<V>())
ekat_gen_where_unary_fcn(log10,invalid<V>())
ekat_gen_where_unary_fcn(tgamma,invalid<V>())
ekat_gen_where_unary_fcn(sqrt,invalid<V>())
ekat_gen_where_unary_fcn(cbrt,invalid<V>())
ekat_gen_where_unary_fcn(tanh,invalid<V>())
ekat_gen_where_unary_fcn(erf,invalid<V>())

#undef ekat_gen_where_unary_fcn

} // namespace ekat

#endif // EKAT_WHERE_HPP
