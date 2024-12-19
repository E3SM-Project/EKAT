#ifndef EKAT_WHERE_HPP
#define EKAT_WHERE_HPP

#include "ekat/ekat_pack.hpp"

namespace ekat
{

// No impl for the general case
template<typename M, typename V>
class where_expression;

// Scalar case: the mask is just a bool
template<typename V>
class where_expression<bool,V>
{
public:
  using mask_t = bool;
  using value_t = V;

  where_expression (const bool& m, V& v)
   : m_mask (m) , m_value (v)
  { /* Nothing to do here */ }

  KOKKOS_FORCEINLINE_FUNCTION
  value_t& value () { return m_value; }

  KOKKOS_FORCEINLINE_FUNCTION
  const mask_t& mask () const { return m_mask; }

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

  bool any  () const { return m_mask; }
  bool all  () const { return m_mask; }
  bool none () const { return !m_mask; }
private:
  const mask_t&  m_mask;
  value_t&       m_value;
};

// Packed case: the mask is an ekat::Mask, and the value is an ekat::Pack

template<int N, typename V>
class where_expression<Mask<N>,Pack<V,N>>
{
public:
  using mask_t = Mask<N>;
  using value_t = Pack<V,N>;

  where_expression (const mask_t& m, value_t& v)
   : m_mask (m) , m_value (v)
  { /* Nothing to do here */ }

  KOKKOS_FORCEINLINE_FUNCTION
  value_t& value () { return m_value; }

  KOKKOS_FORCEINLINE_FUNCTION
  const mask_t& mask () { return m_mask; }

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

  bool any  () const { return m_mask.any(); }
  bool all  () const { return m_mask.all(); }
  bool none () const { return m_mask.none(); }
private:
  const mask_t&  m_mask;
  value_t&       m_value;
};

template<typename S, int N>
where_expression<Mask<N>,Pack<S,N>>
where (const Mask<N>& m, Pack<S,N>& p)
{
  return where_expression<Mask<N>,Pack<S,N>>(m,p);
}

template<typename S>
where_expression<bool,S>
where (const bool& mask, S& scalar)
{
  return where_expression<bool,S>(mask,scalar);
}

} // namespace ekat

#endif // EKAT_WHERE_HPP
