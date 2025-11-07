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

  using ret_left  = eval_t<ELeft>;
  using ret_right = eval_t<ERight>;

  static_assert (expr_l or expr_r,
    "[BinaryExpression] At least one between ELeft and ERight must be an Expression type.\n");

  BinaryExpression (const ELeft& left, const ERight& right)
    : m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  static constexpr int rank () {
    if constexpr (expr_l) {
      if constexpr (expr_r) {
        static_assert(ELeft::rank()==ERight::rank(),
          "[BinaryExpression] Error! ELeft and ERight are Expression types of different rank.\n");
      }
      return ELeft::rank();
    } else {
      return ERight::rank();
    }
  }
  int extent (int i) const {
    if constexpr (expr_l)
      return m_left.extent(i);
    else
      return m_right.extent(i);
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

  static auto ret_type () {
    return std::common_type_t<ret_left,ret_right>(0);
  }
protected:

  KOKKOS_INLINE_FUNCTION
  std::common_type_t<ret_left,ret_right>
  eval_impl (const ret_left& l, const ret_right& r) const {
    using ret_t = std::common_type_t<ret_left,ret_right>;
    if constexpr (OP==BinOp::Plus) {
      return static_cast<const ret_t&>(l)+static_cast<const ret_t&>(r);
    } else if constexpr (OP==BinOp::Minus) {
      return static_cast<const ret_t&>(l)-static_cast<const ret_t&>(r);
    } else if constexpr (OP==BinOp::Mult) {
      return static_cast<const ret_t&>(l)*static_cast<const ret_t&>(r);
    } else if constexpr (OP==BinOp::Div) {
      return static_cast<const ret_t&>(l)/static_cast<const ret_t&>(r);
    } else if constexpr (OP==BinOp::Max) {
      return Kokkos::max(static_cast<const ret_t&>(l),static_cast<const ret_t&>(r));
    } else if constexpr (OP==BinOp::Min) {
      return Kokkos::min(static_cast<const ret_t&>(l),static_cast<const ret_t&>(r));
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
