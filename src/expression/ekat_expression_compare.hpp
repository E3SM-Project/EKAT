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
  static constexpr bool scalar_left  = std::is_arithmetic_v<ELeft>;
  static constexpr bool scalar_right = std::is_arithmetic_v<ERight>;
  static_assert(not scalar_left or not scalar_right,
      "[CmpExpression] One between ELeft and ERight must be non-arithmetic.\n");

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

  int num_indices () const {
    if constexpr (scalar_left) {
      return m_right.num_indices();
    } else if constexpr (scalar_right) {
      return m_left.num_indices();
    } else {
      return std::max(m_left.num_indices(),m_right.num_indices());
    }
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  Real eval(Args... args) const {
    if constexpr (scalar_left) {
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
    } else if constexpr (scalar_right) {
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
protected:

  ELeft    m_left;
  ERight   m_right;

  Comparison m_cmp;
};

template<typename ELeft, typename ERight>
CmpExpression<ELeft,ERight>
cmp_expression (const ELeft& l, const ERight& r, const Comparison CMP)
{
  return CmpExpression<ELeft,ERight>(l,r,CMP);
}

template<typename ELeft, typename ERight>
CmpExpression<ELeft,ERight> operator== (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return CmpExpression<ELeft,ERight>(l.cast(),r.cast(),Comparison::EQ);
}

template<typename ELeft, typename ERight>
CmpExpression<ELeft,ERight> operator!= (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return CmpExpression<ELeft,ERight>(l.cast(),r.cast(),Comparison::NE);
}

template<typename ELeft, typename ERight>
CmpExpression<ELeft,ERight> operator> (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return CmpExpression<ELeft,ERight>(l.cast(),r.cast(),Comparison::GT);
}

template<typename ELeft, typename ERight>
CmpExpression<ELeft,ERight> operator>= (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return CmpExpression<ELeft,ERight>(l.cast(),r.cast(),Comparison::GE);
}

template<typename ELeft, typename ERight>
CmpExpression<ELeft,ERight> operator< (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return CmpExpression<ELeft,ERight>(l.cast(),r.cast(),Comparison::LT);
}

template<typename ELeft, typename ERight>
CmpExpression<ELeft,ERight> operator<= (const Expression<ELeft>& l, const Expression<ERight>& r)
{
  return CmpExpression<ELeft,ERight>(l.cast(),r.cast(),Comparison::LE);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_COMPARE_HPP
