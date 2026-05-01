#ifndef EKAT_EXPRESSION_BINARY_OP_HPP
#define EKAT_EXPRESSION_BINARY_OP_HPP

#include "ekat_expression_meta.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

enum class BinOp {
  Plus,
  Minus,
  Mult,
  Div
};

template<typename ELeft, typename ERight, BinOp OP>
class BinaryExpression {
public:
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  using eval_left_t  = eval_return_t<ELeft>;
  using eval_right_t = eval_return_t<ERight>;

  using eval_t = std::common_type_t<eval_left_t,eval_right_t>;

  // Don't create an expression from builtin types, just combine them!
  static_assert (expr_l or expr_r,
    "[BinaryExpression] At least one between ELeft and ERight must be an Expression type.\n");

  KOKKOS_INLINE_FUNCTION
  BinaryExpression (const ELeft& left, const ERight& right)
    : m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  // Rank: if one sub-expression is rank-0 (scalar result, e.g. a Reduction),
  // the binary expression takes the rank of the other operand.
  // Both must agree on rank when both are non-zero.
  static constexpr int rank () {
    if constexpr (expr_l and expr_r) {
      constexpr int lr = ELeft::rank();
      constexpr int rr = ERight::rank();
      if constexpr (lr > 0 and rr > 0) {
        static_assert(lr == rr,
          "[BinaryExpression] Error! ELeft and ERight are Expression types of different rank.\n");
      }
      return lr > rr ? lr : rr;
    } else if constexpr (expr_l) {
      return ELeft::rank();
    } else {
      return ERight::rank();
    }
  }

  KOKKOS_INLINE_FUNCTION
  int extent (int i) const {
    // Return extent from whichever operand has non-zero rank
    if constexpr (expr_l and expr_r) {
      if constexpr (ELeft::rank() > 0)
        return m_left.extent(i);
      else if constexpr (ERight::rank() > 0)
        return m_right.extent(i);
      else
        return 0;
    } else if constexpr (expr_l) {
      return m_left.extent(i);
    } else {
      return m_right.extent(i);
    }
  }

  // Propagate the highest ExprKind from sub-expressions
  static constexpr ExprKind kind () {
    if constexpr (expr_l and expr_r)
      return expr_kind_max(ELeft::kind(), ERight::kind());
    else if constexpr (expr_l)
      return ELeft::kind();
    else if constexpr (expr_r)
      return ERight::kind();
    else
      return ExprKind::Elemental;
  }

  // Propagate setup to sub-expressions
  template<typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void setup (const MemberType& team) {
    if constexpr (expr_l) m_left.setup(team);
    if constexpr (expr_r) m_right.setup(team);
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  eval_t eval(Args... args) const {
    if constexpr (not expr_l) {
      return eval_impl(m_left,m_right.eval(args...));
    } else if constexpr (not expr_r) {
      return eval_impl(m_left.eval(args...),m_right);
    } else {
      // Both are expressions; handle rank-0 (scalar) operands
      constexpr int lr = ELeft::rank();
      constexpr int rr = ERight::rank();
      if constexpr (lr == 0 and rr > 0) {
        return eval_impl(m_left.eval(),m_right.eval(args...));
      } else if constexpr (rr == 0 and lr > 0) {
        return eval_impl(m_left.eval(args...),m_right.eval());
      } else {
        return eval_impl(m_left.eval(args...),m_right.eval(args...));
      }
    }
  }

protected:

  KOKKOS_INLINE_FUNCTION
  eval_t eval_impl (const eval_left_t& l, const eval_right_t& r) const {
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

// Specialize meta utils
template<typename ELeft, typename ERight, BinOp OP>
struct is_expr<BinaryExpression<ELeft,ERight,OP>> : std::true_type {};
template<typename ELeft, typename ERight, BinOp OP>
struct eval_return<BinaryExpression<ELeft,ERight,OP>> {
  using type = typename BinaryExpression<ELeft,ERight,OP>::eval_t;
};

} // namespace ekat

#endif // EKAT_EXPRESSION_BINARY_OP_HPP
