#ifndef EKAT_EXPRESSION_COMPARE_HPP
#define EKAT_EXPRESSION_COMPARE_HPP

#include "ekat_expression_meta.hpp"

#include "ekat_std_utils.hpp"
#include "ekat_kernel_assert.hpp"
#include "ekat_assert.hpp"

#include <Kokkos_Core.hpp>

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
class CmpExpression {
public:
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  using eval_left_t  = eval_return_t<ELeft>;
  using eval_right_t = eval_return_t<ERight>;
  using eval_t = decltype(std::declval<eval_left_t>()==std::declval<eval_right_t>());

  // Don't create an expression from builtin types, just compare them!
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
    } else if constexpr (expr_r) {
      return ERight::rank();
    } else {
      return 0;
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
  eval_t eval(Args... args) const {
    if constexpr (expr_l) {
      if constexpr (expr_r) {
        return eval_impl(m_left.eval(args...), m_right.eval(args...));
      } else {
        return eval_impl(m_left.eval(args...), m_right);
      }
    } else if constexpr (expr_r) {
      return eval_impl(m_left, m_right.eval(args...));
    } else {
      return eval_impl(m_left, m_right);
    }
  }

protected:

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  eval_t eval_impl(const eval_left_t& l, const eval_right_t& r) const {
    switch (m_cmp) {
      case Comparison::EQ: return l==r;
      case Comparison::NE: return l!=r;
      case Comparison::GT: return l>r;
      case Comparison::GE: return l>=r;
      case Comparison::LT: return l<r;
      case Comparison::LE: return l<=r;
      default:
        EKAT_KERNEL_ERROR_MSG ("Internal error! Unsupported cmp operator.\n");
    }
  }

  ELeft    m_left;
  ERight   m_right;

  Comparison m_cmp;
};

// Specialize meta utils
template<typename ELeft, typename ERight>
struct is_expr<CmpExpression<ELeft,ERight>> : std::true_type {};
template<typename ELeft, typename ERight>
struct eval_return<CmpExpression<ELeft,ERight>> {
  using type = typename CmpExpression<ELeft,ERight>::eval_t;
};

} // namespace ekat

#endif // EKAT_EXPRESSION_COMPARE_HPP
