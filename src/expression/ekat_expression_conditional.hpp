#ifndef EKAT_EXPRESSION_CONDITIONAL_HPP
#define EKAT_EXPRESSION_CONDITIONAL_HPP

#include "ekat_expression_meta.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

template<typename ECond, typename ELeft, typename ERight>
class ConditionalExpression {
public:
  static constexpr bool expr_c = is_expr_v<ECond>;
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  using eval_cond_t  = eval_return_t<ECond>;
  using eval_left_t  = eval_return_t<ELeft>;
  using eval_right_t = eval_return_t<ERight>;

  using eval_t = std::common_type_t<eval_left_t,eval_right_t>;

  // Don't create an expression from builtin types, just use a ternary op!
  static_assert(expr_c or expr_l or expr_r,
    "[CmpExpression] At least one between ECond, ELeft, and ERight must be an Expression type.\n");

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
  eval_t eval (Args... args) const
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
      if (m_cmp)
        if constexpr (expr_l)
          return m_left.eval(args...);
        else
          return m_left;
      else
        if constexpr (expr_r)
          return m_right.eval(args...);
        else
          return m_right;
    }
  }

protected:

  ECond    m_cmp;
  ELeft    m_left;
  ERight   m_right;
};

// Specialize meta utils
template<typename ECond, typename ELeft, typename ERight>
struct is_expr<ConditionalExpression<ECond,ELeft,ERight>> : std::true_type {};
template<typename ECond, typename ELeft, typename ERight>
struct eval_return<ConditionalExpression<ECond,ELeft,ERight>> {
  using type = typename ConditionalExpression<ECond,ELeft,ERight>::eval_t;
};

// Free fcn to construct a ConditionalExpression
template<typename ECond, typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ECond> or is_expr_v<ELeft> or is_expr_v<ERight>,ConditionalExpression<ECond,ELeft,ERight>>
conditional(const ECond& c, const ELeft& l, const ERight& r)
{
  return ConditionalExpression<ECond,ELeft,ERight>(c,l,r);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_CONDITIONAL_HPP
