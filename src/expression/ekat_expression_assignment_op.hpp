#ifndef EKAT_EXPRESSION_ASSIGNMENT_OP_HPP
#define EKAT_EXPRESSION_ASSIGNMENT_OP_HPP

#include "ekat_expression_base.hpp"

#include <iostream>

namespace ekat {

enum class AssignmentOp {
  PlusEq,
  MinusEq,
  MultEq,
  DivEq
};

template<AssignmentOp OP>
constexpr bool is_assignment_op = OP==AssignmentOp::PlusEq or
                                  OP==AssignmentOp::MinusEq or
                                  OP==AssignmentOp::MultEq or
                                  OP==AssignmentOp::DivEq;

template<typename ELeft, typename ERight, AssignmentOp OP>
class AssignmentExpression : public Expression<AssignmentExpression<ELeft,ERight,OP>>{
public:
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;
  static constexpr bool is_assignable = true;

  using ret_left  = eval_t<ELeft>;
  using ret_right = eval_t<ERight>;

  static_assert (expr_l and is_assignable_expr_v<ELeft>,
    "[AssignmentExpression] Error! ELeft MUST be an assignable Expression type.\n");

  AssignmentExpression (const ELeft& left, const ERight& right)
    : m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  static constexpr int rank () {
    if constexpr (expr_r) {
      static_assert(ELeft::rank()==ERight::rank(),
        "[AssignmentExpression] Error! ELeft and ERight are Expression types of different rank.\n");
    }
    return ELeft::rank();
  }
  int extent (int i) const {
    return m_left.extent(i);
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  ret_left& eval(Args... args) const {
    if constexpr (not expr_r) {
      return eval_impl(m_left.access(args...),m_right);
    } else {
      return eval_impl(m_left.access(args...),m_right.eval(args...));
    }
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  ret_left& access(Args... args) const {
    return eval(args...);
  }

  static auto ret_type () { return ret_left(0); }
protected:

  KOKKOS_INLINE_FUNCTION
  ret_left& eval_impl (ret_left& l, const ret_right& r) const {
    if constexpr (OP==AssignmentOp::PlusEq) {
      l += r;
    } else if constexpr (OP==AssignmentOp::MinusEq) {
      l -= r;
    } else if constexpr (OP==AssignmentOp::MultEq) {
      l *= r;
    } else if constexpr (OP==AssignmentOp::DivEq) {
      l /= r;
    }
    return l;
  }

  ELeft    m_left;
  ERight   m_right;
};

template<typename ELeft, typename ERight, AssignmentOp OP>
struct is_expr<AssignmentExpression<ELeft,ERight,OP>> : std::true_type {};
template<typename ELeft, typename ERight, AssignmentOp OP>
struct is_assignable_expr<AssignmentExpression<ELeft,ERight,OP>> : std::true_type {};

// Overload assignment operators
template<typename ELeft, typename ERight>
std::enable_if_t<is_assignable_expr_v<ELeft>,AssignmentExpression<ELeft,ERight,AssignmentOp::PlusEq>>
operator+= (const ELeft& l, const ERight& r)
{
  return AssignmentExpression<ELeft,ERight,AssignmentOp::PlusEq>(l,r);
}
template<typename ELeft, typename ERight>
std::enable_if_t<is_assignable_expr_v<ELeft>,AssignmentExpression<ELeft,ERight,AssignmentOp::MinusEq>>
operator-= (const ELeft& l, const ERight& r)
{
  return AssignmentExpression<ELeft,ERight,AssignmentOp::MinusEq>(l,r);
}
template<typename ELeft, typename ERight>
std::enable_if_t<is_assignable_expr_v<ELeft>,AssignmentExpression<ELeft,ERight,AssignmentOp::MultEq>>
operator*= (const ELeft& l, const ERight& r)
{
  return AssignmentExpression<ELeft,ERight,AssignmentOp::MultEq>(l,r);
}
template<typename ELeft, typename ERight>
std::enable_if_t<is_assignable_expr_v<ELeft>,AssignmentExpression<ELeft,ERight,AssignmentOp::DivEq>>
operator/= (const ELeft& l, const ERight& r)
{
  return AssignmentExpression<ELeft,ERight,AssignmentOp::DivEq>(l,r);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_ASSIGNMENT_OP_HPP

