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
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  static_assert (expr_l or expr_r,
    "[CmpExpression] At least one between ELeft and ERight must be an Expression type.\n");

  BinaryExpression (const ELeft& left, const ERight& right)
    : m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  int num_indices () const {
    if constexpr (not expr_l) {
      return m_right.num_indices();
    } else if constexpr (not expr_r) {
      return m_left.num_indices();
    } else {
      return std::max(m_left.num_indices(),m_right.num_indices());
    }
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  auto eval(Args... args) const {
    if constexpr (not expr_l) {
      return eval_impl(m_left,m_right.eval(args...));
    } else if constexpr (not expr_r) {
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
    } else if constexpr (OP==BinOp::Min) {
      return Kokkos::min(l,r);
    }
  }

  ELeft    m_left;
  ERight   m_right;
};

template<typename ELeft, typename ERight, BinOp OP>
struct is_expr<BinaryExpression<ELeft,ERight,OP>> : std::true_type {};

// Unary minus implemented as -1*expr
template<typename ERight>
BinaryExpression<int,ERight,BinOp::Mult>
operator- (const Expression<ERight>& r)
{
  return BinaryExpression<int,ERight,BinOp::Mult>(-1,r.cast());
}

// Overload arithmetic operators
template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Plus>>
operator+ (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Plus>(l,r);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Minus>>
operator- (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Minus>(l,r);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Mult>>
operator* (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Mult>(l,r);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Div>>
operator/ (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Div>(l,r);
}

// Overload max/min functions
template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Max>>
max (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Max>(l,r);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Min>>
min (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Min>(l,r);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_BINARY_OP_HPP
