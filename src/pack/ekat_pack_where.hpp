#ifndef EKAT_PACK_WHERE_HPP
#define EKAT_PACK_WHERE_HPP

#include "ekat_where.hpp"

namespace ekat
{

template<int N, typename V>
class where_expression<Pack<V,N>>
{
public:
  using mask_t = Mask<N>;
  using value_t = Pack<V,N>;
  using scalar_t = typename ekat::ScalarTraits<value_t>::scalar_type;

  KOKKOS_FORCEINLINE_FUNCTION
  where_expression (const mask_t& m, value_t& v)
   : m_mask (m) , m_value (v)
  { /* Nothing to do here */ }

  KOKKOS_FORCEINLINE_FUNCTION
        value_t& value ()       { return m_value; }
  KOKKOS_FORCEINLINE_FUNCTION
  const value_t& value () const { return m_value; }

  KOKKOS_FORCEINLINE_FUNCTION
  const mask_t& mask () const { return m_mask; }

  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator=(const S rhs)  {
    m_value.set(m_mask,rhs);
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator=(const Pack<S,N>& rhs)  {
    m_value.set(m_mask,rhs);
    return m_value;
  }

  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator+=(const S rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] += rhs;
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator+=(const Pack<S,N>& rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] += rhs[i];
    return m_value;
  }

  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator-=(const S rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] -= rhs;
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator-=(const Pack<S,N>& rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] -= rhs[i];
    return m_value;
  }

  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator*=(const S rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] *= rhs;
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator*=(const Pack<S,N>& rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] *= rhs[i];
    return m_value;
  }

  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator/=(const S rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] /= rhs;
    return m_value;
  }
  template<typename S>
  KOKKOS_FORCEINLINE_FUNCTION
  typename std::enable_if<std::is_convertible<V,S>::value,value_t&>::type
  operator/=(const Pack<S,N>& rhs)  {
    ekat_masked_loop(m_mask,i)
      m_value[i] /= rhs[i];
    return m_value;
  }

  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t max (const scalar_t& v) const {
    return ekat::max(m_mask,v,m_value);
  }

  KOKKOS_FORCEINLINE_FUNCTION
  scalar_t min (const scalar_t& v) const {
    return ekat::min(m_mask,v,m_value);
  }

  KOKKOS_FORCEINLINE_FUNCTION
  bool any  () const { return m_mask.any(); }
  KOKKOS_FORCEINLINE_FUNCTION
  bool all  () const { return m_mask.all(); }
  KOKKOS_FORCEINLINE_FUNCTION
  bool none () const { return m_mask.none(); }
private:
  const mask_t   m_mask;
  value_t&       m_value;
};

// ----- Create where expressions from mask and pack objects ------ //

template<typename S, int N>
KOKKOS_FORCEINLINE_FUNCTION
where_expression<Pack<S,N>>
where (const Mask<N>& m, Pack<S,N>& p)
{
  return where_expression<Pack<S,N>>(m,p);
}

// --------- Allow to do reductions between scalars and where expression ------- //

#define scalar_where_update_op(op)                                \
template<typename V, int N>                                       \
KOKKOS_FORCEINLINE_FUNCTION                                       \
typename where_expression<Pack<V,N>>::scalar_t&                   \
operator op (typename where_expression<Pack<V,N>>::scalar_t& lhs, \
             const where_expression<Pack<V,N>>& rhs)              \
{                                                                 \
  ekat_masked_loop(rhs.mask(),i)                                  \
    lhs op rhs.value()[i];                                        \
  return lhs;                                                     \
}

scalar_where_update_op(+=)
scalar_where_update_op(-=)
scalar_where_update_op(*=)
scalar_where_update_op(/=)

#undef scalar_where_update_op

} // namespace ekat

#endif // EKAT_PACK_WHERE_HPP
