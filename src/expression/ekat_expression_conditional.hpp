#ifndef EKAT_EXPRESSION_CONDITIONAL_HPP
#define EKAT_EXPRESSION_CONDITIONAL_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

template<typename ECond, typename ELeft, typename ERight>
class ConditionalExpression : public ExpressionBase<ConditionalExpression<ECond,ELeft,ERight>> {
public:
  static constexpr bool expr_c = is_expr_v<ECond>;
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  using return_cond_t  = eval_return_t<ECond>;
  using return_left_t  = eval_return_t<ELeft>;
  using return_right_t = eval_return_t<ERight>;

  using return_type = std::common_type_t<return_left_t,return_right_t>;

  // Don't create an expression from builtin types, just use a ternary op!
  static_assert(expr_c or expr_l or expr_r,
    "[ConditionalExpression] At least one between ECond, ELeft, and ERight must be an Expression type.\n");

  ConditionalExpression (const ECond& cmp, const ELeft& left, const ERight& right)
    : m_cmp(cmp)
    , m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  static constexpr int rank() {
    if constexpr (expr_c) {
      if constexpr (expr_l) {
        static_assert(ECond::rank()==ELeft::rank(),
          "[ConditionalExpression] Error! ECond and ELeft are Expression types of different rank.\n");
      }
      if constexpr (expr_r) {
        static_assert(ECond::rank()==ERight::rank(),
          "[ConditionalExpression] Error! ECond and ERight are Expression types of different rank.\n");
      }
      return ECond::rank();
    } else if constexpr (expr_l) {
      if constexpr (expr_r) {
        static_assert(ELeft::rank()==ERight::rank(),
          "[ConditionalExpression] Error! ELeft and ERight are Expression types of different rank.\n");
      }
      return ELeft::rank();
    } else {
      return ERight::rank();
    }
  }
  int extent (int i) const {
    if constexpr (expr_c)
      return m_cmp.extent(i);
    else if constexpr (expr_l)
      return m_left.extent(i);
    else
      return m_right.extent(i);
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  return_type eval (Args... args) const
  {
    if constexpr (expr_c) {
      if (m_cmp.eval(args...))
        if constexpr (expr_l)
          return m_left.eval(args...);
        else
          return m_left;
      else
        if constexpr (expr_r)
          return m_right.eval(args...);
        else
          return m_right;
    } else {
      if (m_cmp) {
        if constexpr (expr_l)
          return m_left.eval(args...);
        else
          return m_left;
      } else {
        if constexpr (expr_r)
          return m_right.eval(args...);
        else
          return m_right;
      }
    }
  }

protected:

  ECond    m_cmp;
  ELeft    m_left;
  ERight   m_right;
};

// Free fcn to construct a ConditionalExpression
template<typename TC, typename T1, typename T2,
         typename = std::enable_if_t<is_any_expr_v<TC, T1, T2>>>
auto if_then_else(const TC& c, const T1& l, const T2& r)
{
  using  ret_t = ConditionalExpression<get_expr_node_t<TC>,
                                       get_expr_node_t<T1>,
                                       get_expr_node_t<T2>>;

  return ret_t(get_expr_node(c),get_expr_node(l),get_expr_node(r));
}

} // namespace ekat

#endif // EKAT_EXPRESSION_CONDITIONAL_HPP
