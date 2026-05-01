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
class CmpExpression : public ExpressionBase<CmpExpression<ELeft,ERight>> {
public:
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  using return_left_t  = eval_return_t<ELeft>;
  using return_right_t = eval_return_t<ERight>;
  using return_type = decltype(std::declval<return_left_t>()==std::declval<return_right_t>());

  // Don't create an expression from builtin types, just compare them!
  static_assert(expr_l or expr_r,
    "[CmpExpression] At least one between ELeft and ERight must be an Expression type.\n");

  CmpExpression (const ELeft& left,
                 const ERight& right,
                 Comparison CMP)
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
        static_assert(ELeft::rank()==ERight::rank() or ELeft::rank()==0 or ERight::rank()==0,
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
  return_type eval(Args... args) const {
    if constexpr (expr_l) {
      if constexpr (expr_r)
        return eval_impl(m_left.eval(args...), m_right.eval(args...));
      else
        return eval_impl(m_left.eval(args...), m_right);
    } else if constexpr (expr_r) {
      return eval_impl(m_left, m_right.eval(args...));
    } else {
      return eval_impl(m_left, m_right);
    }
  }

protected:

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  return_type eval_impl(const return_left_t& l, const return_right_t& r) const {
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

// Overload comparison operators
#define EKAT_GEN_CMP_OP_EXPR(OP,ENUM)                                 \
  template<typename T1, typename T2,                                  \
           typename = std::enable_if_t<is_any_expr_v<T1,T2>>>         \
  KOKKOS_INLINE_FUNCTION                                              \
  auto operator OP (const T1& l, const T2& r)                         \
  {                                                                   \
    using  ret_t = CmpExpression<get_expr_node_t<T1>,                 \
                                 get_expr_node_t<T2>>;                \
                                                                      \
    return ret_t(get_expr_node(l),get_expr_node(r),Comparison::ENUM); \
  }

EKAT_GEN_CMP_OP_EXPR(==,EQ);
EKAT_GEN_CMP_OP_EXPR(!=,NE);
EKAT_GEN_CMP_OP_EXPR(> ,GT);
EKAT_GEN_CMP_OP_EXPR(>=,GE);
EKAT_GEN_CMP_OP_EXPR(< ,LT);
EKAT_GEN_CMP_OP_EXPR(<=,LE);

#undef EKAT_GEN_CMP_OP_EXPR

} // namespace ekat

#endif // EKAT_EXPRESSION_COMPARE_HPP
