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
template<typename V>
class where_expression
{
public:
  using mask_t = bool;
  using value_t = V;
  using scalar_traits = ekat::ScalarTraits<value_t>;
  using scalar_t = typename scalar_traits::scalar_type;

  static_assert (not scalar_traits::is_simd,
      "Error! Base impl of where_expression only works for non-simd types.\n");

  KOKKOS_FORCEINLINE_FUNCTION
  where_expression (const bool& m, V& v)
   : m_mask (m) , m_value (v)
  { /* Nothing to do here */ }

  KOKKOS_FORCEINLINE_FUNCTION
        value_t& value ()       { return m_value; }
  KOKKOS_FORCEINLINE_FUNCTION
  const value_t& value () const { return m_value; }

  KOKKOS_FORCEINLINE_FUNCTION
  const mask_t& mask () const { return m_mask; }

  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t max (const scalar_t& v) const {
    if (m_mask)
      return impl::max(v,m_value);
    else
      return v;
  }
  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t min (const scalar_t& v) const {
    if (m_mask)
      return impl::min(v,m_value);
    else
      return v;
  }

  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  value_t& operator=(const S rhs)  {
    if (m_mask) m_value=rhs;
    return m_value;
  }

  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  value_t& operator+=(const S rhs)  {
    if (m_mask) m_value+=rhs;
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  value_t& operator-=(const S rhs)  {
    if (m_mask) m_value-=rhs;
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  value_t& operator*=(const S rhs)  {
    if (m_mask) m_value*=rhs;
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  value_t& operator/=(const S rhs)  {
    if (m_mask) m_value/=rhs;
    return m_value;
  }

  KOKKOS_FORCEINLINE_FUNCTION
  bool any  () const { return m_mask; }
  KOKKOS_FORCEINLINE_FUNCTION
  bool all  () const { return m_mask; }
  KOKKOS_FORCEINLINE_FUNCTION
  bool none () const { return !m_mask; }
private:
  const mask_t   m_mask;
  value_t&       m_value;
};

// ----- Create where expressions from mask-like and value objects ------ //

template<typename S>
KOKKOS_FORCEINLINE_FUNCTION
where_expression<S>
where (const bool& mask, S& scalar)
{
  return where_expression<S>(mask,scalar);
}

// --------- Allow to do reductions between scalars and where expression ------- //

template<typename V>
KOKKOS_FORCEINLINE_FUNCTION
typename where_expression<V>::scalar_t
max (const where_expression<V>& w, const typename where_expression<V>::scalar_t& s)
{
  return w.max(s);
}

template<typename V>
KOKKOS_FORCEINLINE_FUNCTION
typename where_expression<V>::scalar_t
min (const where_expression<V>& w, const typename where_expression<V>::scalar_t& s)
{
  return w.min(s);
}

#define scalar_where_update_op(op)                                \
template<typename V>                                              \
KOKKOS_FORCEINLINE_FUNCTION                                       \
typename where_expression<V>::scalar_t&                           \
operator op (typename where_expression<V>::scalar_t& lhs,         \
             const where_expression<V>& rhs)                      \
{                                                                 \
  if (rhs.mask()) lhs op rhs.value();                             \
  return lhs;                                                     \
}                                                                 \

scalar_where_update_op(+=)
scalar_where_update_op(-=)
scalar_where_update_op(*=)
scalar_where_update_op(/=)

#undef scalar_where_update_op

} // namespace ekat

#endif // EKAT_WHERE_HPP
