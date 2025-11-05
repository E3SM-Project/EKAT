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
  static constexpr bool scalar_l = std::is_arithmetic_v<ELeft>;
  static constexpr bool scalar_r = std::is_arithmetic_v<ERight>;

  static_assert (not (scalar_l and scalar_r),
    "[BinaryExpression] Error! At least one of the two terms must be an Expression instance.\n");

  BinaryExpression (const ELeft& left, const ERight& right)
    : m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  int num_indices () const { return std::max(m_left.num_indices(),m_right.num_indices()); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  auto eval(Args... args) const {
    if constexpr (scalar_l) {
      return eval_impl(m_left,m_right.eval(args...));
    } else if constexpr (scalar_r) {
      return eval_impl(m_left.eval(args...),m_right);
    } else {
      return eval_impl(m_left.eval(args...),m_right.eval(args...));
    }
  }

  static auto ret_type () { return ELeft::ret_type() + ERight::ret_type(); }
protected:

  template<typename T1, typename T2>
  KOKKOS_INLINE_FUNCTION
  auto eval_impl(const T1 l, const T2 r) const {
    if constexpr (OP==BinOp::Plus) {
      return l+r;
    } else if constexpr (OP==BinOp::Minus) {
      return l-r;
    } else if constexpr (OP==BinOp::Mult) {
      return l*r;
    } else if constexpr (OP==BinOp::Div) {
      return l/r;
    } else if constexpr (OP==BinOp::Max) {
      return Kokkos::max(l,r);
    } else if constexpr (OP==BinOp::Div) {
      return Kokkos::min(l,r);
    }
  }

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
