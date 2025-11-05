#ifndef EKAT_EXPRESSION_BINARY_OP_HPP
#define EKAT_EXPRESSION_BINARY_OP_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

enum class BinOp {
  Plus,
  Minus,
  Mult,
  Div,
  Max,
  Min
};

template<typename ELeft, typename ERight, BinOp OP>
class BinaryExpression : public Expression<BinaryExpression<ELeft,ERight,OP>>{
public:
  BinaryExpression (const ELeft& left, const ERight& right)
    : m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  int num_indices () const { return std::max(m_left.num_indices(),m_right.num_indices()); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  Real eval(Args... args) const {
    if constexpr (OP==BinOp::Plus) {
      return m_left.eval(args...) + m_right.eval(args...);
    } else if constexpr (OP==BinOp::Minus) {
      return m_left.eval(args...) - m_right.eval(args...);
    } else if constexpr (OP==BinOp::Mult) {
      return m_left.eval(args...) * m_right.eval(args...);
    } else if constexpr (OP==BinOp::Div) {
      return m_left.eval(args...) / m_right.eval(args...);
    } else if constexpr (OP==BinOp::Max) {
      return Kokkos::max(m_left.eval(args...),m_right.eval(args...));
    } else if constexpr (OP==BinOp::Div) {
      return Kokkos::min(m_left.eval(args...),m_right.eval(args...));
    }
  }
protected:

  ELeft    m_left;
  ERight   m_right;
};

template<typename ELeft, typename ERight>
BinaryExpression<ELeft,ERight,BinOp::Plus>
operator+ (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Plus>(l.cast(),r.cast());
}

template<typename ELeft, typename ERight>
BinaryExpression<ELeft,ERight,BinOp::Minus>
operator- (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Minus>(l.cast(),r.cast());
}

template<typename ELeft, typename ERight>
BinaryExpression<ELeft,ERight,BinOp::Mult>
operator* (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Mult>(l.cast(),r.cast());
}

template<typename ELeft, typename ERight>
BinaryExpression<ELeft,ERight,BinOp::Div>
operator/ (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Div>(l.cast(),r.cast());
}

} // namespace ekat

#endif // EKAT_EXPRESSION_BINARY_OP_HPP
