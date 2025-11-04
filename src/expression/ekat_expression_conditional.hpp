#ifndef EKAT_EXPRESSION_CONDITIONAL_HPP
#define EKAT_EXPRESSION_CONDITIONAL_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

template<typename ECond, typename ELeft, typename ERight>
class ConditionalExpression : public Expression<ConditionalExpression<ECond,ELeft,ERight>> {
public:
  ConditionalExpression (const ECond& cmp, const ELeft& left, const ERight& right)
    : m_cmp(cmp)
    , m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  int num_indices () const {
    return std::max(m_cmp.num_indices(),std::max(m_left.num_indices(),m_right.num_indices()));
  }

  KOKKOS_INLINE_FUNCTION
  Real eval (int i) const {
    if (m_cmp.eval(i))
      return m_left.eval(i);
    else
      return m_right.eval(i);
  }
  KOKKOS_INLINE_FUNCTION
  Real eval (int i, int j) const {
    if (m_cmp.eval(i,j))
      return m_left.eval(i,j);
    else
      return m_right.eval(i,j);
  }
  KOKKOS_INLINE_FUNCTION
  Real eval (int i, int j, int k) const {
    if (m_cmp.eval(i,j,k))
      return m_left.eval(i,j,k);
    else
      return m_right.eval(i,j,k);
  }
protected:

  ECond    m_cmp;
  ELeft    m_left;
  ERight   m_right;
};

template<typename ECond, typename ELeft, typename ERight>
ConditionalExpression<ECond,ELeft,ERight>
conditional(const Expression<ECond>& c, const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return ConditionalExpression<ECond,ELeft,ERight>(c.cast(),l.cast(),r.cast());
}

} // namespace ekat

#endif // EKAT_EXPRESSION_CONDITIONAL_HPP
