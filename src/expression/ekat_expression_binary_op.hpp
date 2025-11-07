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

  BinaryExpression (const ELeft& left, const ERight& right)
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
  eval_t eval(Args... args) const {
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
  eval_t eval_impl (const eval_left_t& l, const eval_right_t& r) const {
    if constexpr (OP==BinOp::Plus) {
      return l+r;
    } else if constexpr (OP==BinOp::Minus) {
      return l-r;
    } else if constexpr (OP==BinOp::Mult) {
      return l*r;
    } else if constexpr (OP==BinOp::Div) {
      return l/r;
      return Kokkos::min(static_cast<const eval_t&>(l),static_cast<const eval_t&>(r));
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
