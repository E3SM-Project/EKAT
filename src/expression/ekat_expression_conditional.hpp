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
  using ret_t = std::common_type_t<ret_left,ret_right>;

  static_assert(expr_c or expr_l or expr_r,
    "[CmpExpression] At least one between ECond, ELeft, and ERight must be an Expression type.\n");

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

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  ret_t eval (Args... args) const
  {
    if constexpr (expr_r) {
      ret_t r = m_right.eval(args...);
      if constexpr (expr_c) {
        auto w = where(m_cmp.eval(args...),r);
        if constexpr (expr_l)
          w = m_left.eval(args...);
        else
          w = m_left;
        return w.value();
      } else {
        auto w = where(m_cmp,r);
        if constexpr (expr_l)
          w = m_left.eval(args...);
        else
          w = m_left;
        return w.value();
      }
    } else {
      ret_t r = m_right;
      if constexpr (expr_c) {
        auto w = where(m_cmp.eval(args...),r);
        if constexpr (expr_l)
          w = m_left.eval(args...);
        else
          w = m_left;
        return w.value();
      } else {
        auto w = where(m_cmp,r);
        if constexpr (expr_l)
          w = m_left.eval(args...);
        else
          w = m_left;
        return w.value();
      }
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
struct is_expr<ConditionalExpression<ECond,ELeft,ERight>> : std::true_type {};

template<typename ECond, typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ECond> or is_expr_v<ELeft> or is_expr_v<ERight>,ConditionalExpression<ECond,ELeft,ERight>>
conditional(const ECond& c, const ELeft& l, const ERight& r)
{
  return ConditionalExpression<ECond,ELeft,ERight>(c,l,r);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_CONDITIONAL_HPP
