#ifndef EKAT_EXPRESSION_BINARY_OP_HPP
#define EKAT_EXPRESSION_BINARY_OP_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

enum class BinOp {
  Plus,
  Minus,
  Mult,
  Div
};

template<typename ELeft, typename ERight, BinOp OP>
class BinaryExpression : public ExpressionBase<BinaryExpression<ELeft,ERight,OP>> {
public:
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;
  static_assert (expr_l or expr_r, "[BinaryExpression] Error! At least one operand must be an Expression type.\n");

  using return_left_t  = eval_return_t<ELeft>;
  using return_right_t = eval_return_t<ERight>;

  using return_type = std::common_type_t<return_left_t,return_right_t>;

  BinaryExpression (const ELeft& left,
                    const ERight& right)
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
  return_type eval(Args... args) const {
    if constexpr (not expr_l) {
      return eval_impl(m_left,m_right.eval(args...));
    } else if constexpr (not expr_r) {
      return eval_impl(m_left.eval(args...),m_right);
    } else {
      return eval_impl(m_left.eval(args...),m_right.eval(args...));
    }
  }

protected:

  KOKKOS_INLINE_FUNCTION
  return_type eval_impl (const return_left_t& l, const return_right_t& r) const {
    if constexpr (OP==BinOp::Plus) {
      return l+r;
    } else if constexpr (OP==BinOp::Minus) {
      return l-r;
    } else if constexpr (OP==BinOp::Mult) {
      return l*r;
    } else {
      return l/r;
    }
  }

  ELeft    m_left;
  ERight   m_right;
};

// We could impl op- via BinaryOp (with -1*Expr), but a dedicated class is easier
template<typename EInner>
class NegateExpression : public ExpressionBase<NegateExpression<EInner>> {
public:

  using return_type = eval_return_t<EInner>;

  NegateExpression (const ExpressionBase<EInner>& inner)
   : m_inner(inner.cast())
  {
    // Nothing to do here
  }

  static constexpr int rank () { return EInner::rank(); }
  int extent (int i) const { return m_inner.extent(i); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  return_type eval(Args... args) const {
    return -m_inner.eval(args...);
  }

protected:
  EInner    m_inner;
};

// Unary minus implemented as -1*expr
template<typename ERight>
KOKKOS_INLINE_FUNCTION
auto operator- (const ExpressionBase<ERight>& r)
{
  return NegateExpression(r);
}

// Overload arithmetic operators
#define EKAT_GEN_BIN_OP_EXPR(OP,ENUM)                           \
  template<typename T1, typename T2,                            \
           typename = std::enable_if_t<is_any_expr_v<T1,T2>>>   \
  KOKKOS_INLINE_FUNCTION                                        \
  auto operator OP (const T1& l, const T2& r)                   \
  {                                                             \
    using  ret_t = BinaryExpression<get_expr_node_t<T1>,        \
                                    get_expr_node_t<T2>,        \
                                    BinOp::ENUM>;               \
                                                                \
    return ret_t(get_expr_node(l),get_expr_node(r));            \
  }

EKAT_GEN_BIN_OP_EXPR(+,Plus);
EKAT_GEN_BIN_OP_EXPR(-,Minus);
EKAT_GEN_BIN_OP_EXPR(*,Mult);
EKAT_GEN_BIN_OP_EXPR(/,Div);

#undef EKAT_GEN_BIN_OP_EXPR

} // namespace ekat

#endif // EKAT_EXPRESSION_BINARY_OP_HPP
