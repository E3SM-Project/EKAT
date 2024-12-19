#ifndef EKAT_SIMD_UTILS_HPP
#define EKAT_SIMD_UTILS_HPP

namespace ekat
{

template<typename M, typename V>
class where_expression
{
public:
  where_expression (const M& m, V& v)
   : m_mask (m)
   , m_value (v)
  {
    // Nothing to do here
  }

  KOKKOS_FORCEINLINE_FUNCTION
  V& value () { return m_value; }
  KOKKOS_FORCEINLINE_FUNCTION
  const M& mask () { return m_mask; }
private:
  V&        m_value;
  const M&  m_mask;
};

template<typename S, int N>
where_expression<Mask<N>,Pack<S,N>>
where (const Mask<N>& m, Pack<S,N>& p)
{
  return where_expression(m,p);
}

template<typename S>
where_expression<bool,S>>
where (const bool& mask, S& scalar)
{
  return where_expression(mask,scalar);
}

#define ekat_where_bin_op_ss(op) \
  template<typename S, typename T> \
  S& \
  operator op (where_expression<bool,S>& lhs, const T rhs) \
  { \
    if (lhs.mask()) \
      lhs.value () op rhs; \
  }

#define ekat_where_bin_op_ps(op) \
  template<typename S, int N, typename T> \
  Pack<S,N>& \
  operator op (where_expression<Mask<N>,Pack<S,N>>& lhs, const T rhs) \
  { \
    ekat_masked_loop(lhs.mask(),i) { \
      lhs.value()[i] op rhs; \
    } \
  }

#define ekat_where_bin_op_pp(op) \
  template<typename S, int N, typename T> \
  Pack<S,N>&                                                            \
  operator op (where_expression<Mask<N>,Pack<S,N>>& lhs, const Pack<T,N>& rhs) \
  { \
    ekat_masked_loop(lhs.mask(),i) { \
      lhs.value()[i] op rhs[i]; \
    } \
  } \
  template<typename S, int N, typename T> \
  Pack<S,N>& \
  operator op (Pack<T,N>& lhs, where_expression<Mask<N>,Pack<S,N>>& rhs) \
  { \
    ekat_masked_loop(rhs.mask(),i) { \
      lhs[i] op rhs.value()[i]; \
    } \
  } \

#define ekat_where_bin_op(op) \
  ekat_where_bin_op_ss(op)  \
  ekat_where_bin_op_ps(op)  \
  ekat_where_bin_op_pp(op)

ekat_where_bin_op(+=)
ekat_where_bin_op(-=)
ekat_where_bin_op(*=)
ekat_where_bin_op(/=)

} // namespace ekat

where(vort>2,tend) += temp + vertv

#endif // EKAT_SIMD_UTILS_HPP
