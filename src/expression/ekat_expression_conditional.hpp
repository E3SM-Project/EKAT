#ifndef EKAT_EXPRESSION_CONDITIONAL_HPP
#define EKAT_EXPRESSION_CONDITIONAL_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

template<typename ECond, typename ELeft, typename ERight>
class ConditionalExpression : public Expression<ConditionalExpression<ECond,ELeft,ERight>> {
public:
  static constexpr bool expr_c = is_expr_v<ECond>;
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  using ret_left  = std::conditional_t<expr_l,decltype(ELeft::ret_type()),ELeft>;
  using ret_right = std::conditional_t<expr_r,decltype(ERight::ret_type()),ERight>;

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

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  std::common_type_t<ret_left,ret_right> eval (Args... args) const
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

  static auto ret_type () {
    auto ret_l = ELeft::ret_type();
    auto ret_r = ERight::ret_type();
    using type = std::common_type_t<decltype(ret_l),decltype(ret_r)>;
    return type(0);
  }
protected:

  ECond    m_cmp;
  ELeft    m_left;
  ERight   m_right;
};

template<typename ECond, typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ECond> or is_expr_v<ELeft> or is_expr_v<ERight>,ConditionalExpression<ECond,ELeft,ERight>>
conditional(const ECond& c, const ELeft& l, const ERight& r)
{
  return ConditionalExpression<ECond,ELeft,ERight>(c,l,r);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_CONDITIONAL_HPP
