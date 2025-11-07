#ifndef EKAT_EXPRESSION_COMPARE_HPP
#define EKAT_EXPRESSION_COMPARE_HPP

#include "ekat_expression_base.hpp"

#include "ekat_std_utils.hpp"
#include "ekat_kernel_assert.hpp"
#include "ekat_assert.hpp"

namespace ekat {

enum class Comparison : int {
  EQ,   // ==
  NE,   // !=
  GT,   // >
  GE,   // >=
  LT,   // <
  LE    // <=
};

template<typename ELeft, typename ERight>
class CmpExpression : public Expression<CmpExpression<ELeft,ERight>> {
public:
  using ret_t = int;

  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  static_assert(expr_l or expr_r,
    "[CmpExpression] At least one between ELeft and ERight must be an Expression type.\n");

  CmpExpression (const ELeft& left, const ERight& right, Comparison CMP)
    : m_left(left)
    , m_right(right)
    , m_cmp(CMP)
  {
    auto valid = {Comparison::EQ,Comparison::NE,Comparison::GT,
                  Comparison::GE,Comparison::LT,Comparison::LE};
    EKAT_REQUIRE_MSG (ekat::contains(valid,CMP),
      "[CmpExpression] Error! Unrecognized/unsupported Comparison value.\n");
  }

  static constexpr int rank() {
    if constexpr (expr_l) {
      if constexpr (expr_r) {
        static_assert(ELeft::rank()==ERight::rank(),
          "[CmpExpression] Error! ELeft and ERight are Expression types of different rank.\n");
      }
      return ELeft::rank();
    } else {
      return ERight::rank();
    }
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  ret_t eval(Args... args) const {
    if constexpr (not expr_l) {
      switch (m_cmp) {
        case Comparison::EQ: return m_left == m_right.eval(args...);
        case Comparison::NE: return m_left != m_right.eval(args...);
        case Comparison::GT: return m_left >  m_right.eval(args...);
        case Comparison::GE: return m_left >= m_right.eval(args...);
        case Comparison::LT: return m_left <  m_right.eval(args...);
        case Comparison::LE: return m_left <= m_right.eval(args...);
        default:
          EKAT_KERNEL_ERROR_MSG ("Internal error! Unsupported cmp operator.\n");
      }
    } else if constexpr (not expr_r) {
      switch (m_cmp) {
        case Comparison::EQ: return m_left.eval(args...) == m_right;
        case Comparison::NE: return m_left.eval(args...) != m_right;
        case Comparison::GT: return m_left.eval(args...) >  m_right;
        case Comparison::GE: return m_left.eval(args...) >= m_right;
        case Comparison::LT: return m_left.eval(args...) <  m_right;
        case Comparison::LE: return m_left.eval(args...) <= m_right;
        default:
          EKAT_KERNEL_ERROR_MSG ("Internal error! Unsupported cmp operator.\n");
      }
    } else {
      switch (m_cmp) {
        case Comparison::EQ: return m_left.eval(args...) == m_right.eval(args...);
        case Comparison::NE: return m_left.eval(args...) != m_right.eval(args...);
        case Comparison::GT: return m_left.eval(args...) >  m_right.eval(args...);
        case Comparison::GE: return m_left.eval(args...) >= m_right.eval(args...);
        case Comparison::LT: return m_left.eval(args...) <  m_right.eval(args...);
        case Comparison::LE: return m_left.eval(args...) <= m_right.eval(args...);
        default:
          EKAT_KERNEL_ERROR_MSG ("Internal error! Unsupported cmp operator.\n");
      }
    }
  }

  static int ret_type () { return 0; }
protected:

  ELeft    m_left;
  ERight   m_right;

  Comparison m_cmp;
};

template<typename ELeft, typename ERight>
struct is_expr<CmpExpression<ELeft,ERight>> : std::true_type {};

// Overload cmp operators for Expression types
template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,CmpExpression<ELeft,ERight>>
operator== (const ELeft& l, const ERight& r)
{
  return CmpExpression<ELeft,ERight>(l,r,Comparison::EQ);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,CmpExpression<ELeft,ERight>>
operator!= (const ELeft& l, const ERight& r)
{
  return CmpExpression<ELeft,ERight>(l,r,Comparison::NE);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,CmpExpression<ELeft,ERight>>
operator> (const ELeft& l, const ERight& r)
{
  return CmpExpression<ELeft,ERight>(l,r,Comparison::GT);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,CmpExpression<ELeft,ERight>>
operator>= (const ELeft& l, const ERight& r)
{
  return CmpExpression<ELeft,ERight>(l,r,Comparison::GE);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,CmpExpression<ELeft,ERight>>
operator< (const ELeft& l, const ERight& r)
{
  return CmpExpression<ELeft,ERight>(l,r,Comparison::LT);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,CmpExpression<ELeft,ERight>>
operator<= (const ELeft& l, const ERight& r)
{
  return CmpExpression<ELeft,ERight>(l,r,Comparison::LE);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_COMPARE_HPP
