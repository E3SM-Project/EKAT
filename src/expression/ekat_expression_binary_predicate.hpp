#ifndef EKAT_EXPRESSION_BINARY_PREDICATE_HPP
#define EKAT_EXPRESSION_BINARY_PREDICATE_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

enum class BinaryPredicateOp : int {
  EQ,   // ==
  NE,   // !=
  GT,   // >
  GE,   // >=
  LT,   // <
  LE,   // <=
  AND,  // logical and
  OR    // logical or
};

template<typename ELeft, typename ERight, BinaryPredicateOp Op>
class BinaryPredicateExpression : public ExpressionBase<BinaryPredicateExpression<ELeft,ERight,Op>> {
public:
  static constexpr bool expr_l = is_expr_v<ELeft>;
  static constexpr bool expr_r = is_expr_v<ERight>;

  using return_left_t  = eval_return_t<ELeft>;
  using return_right_t = eval_return_t<ERight>;
  // The return type is a logical-like, but same for all Op's, so just use one
  using return_type = decltype(std::declval<return_left_t>()==std::declval<return_right_t>());

  // Don't create an expression from builtin types, just compare them!
  static_assert(expr_l or expr_r,
    "[BinaryPredicateExpression] At least one between ELeft and ERight must be an Expression type.\n");

  BinaryPredicateExpression (const ELeft& left,
                       const ERight& right)
    : m_left(left)
    , m_right(right)
  {
    // Nothing to do here
  }

  static constexpr int rank() {
    if constexpr (expr_l) {
      if constexpr (expr_r) {
        static_assert(ELeft::rank()==ERight::rank(),
          "[BinaryPredicateExpression] Error! ELeft and ERight are Expression types of different rank.\n");
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
    if constexpr (Op==BinaryPredicateOp::EQ)
      return l==r;
    else if constexpr (Op==BinaryPredicateOp::NE)
      return l!=r;
    else if constexpr (Op==BinaryPredicateOp::GT)
      return l>r;
    else if constexpr (Op==BinaryPredicateOp::GE)
      return l>=r;
    else if constexpr (Op==BinaryPredicateOp::LT)
      return l<r;
    else if constexpr (Op==BinaryPredicateOp::LE)
      return l<=r;
    else if constexpr (Op==BinaryPredicateOp::AND)
      return l and r;
    else
      return l or r;
  }

  ELeft    m_left;
  ERight   m_right;
};

// Overload comparison operators
#define EKAT_GEN_BIN_PREDICATE_EXPR(OP,ENUM)                            \
  template<typename T1, typename T2,                                    \
           typename = std::enable_if_t<is_any_expr_v<T1,T2>>>           \
  KOKKOS_INLINE_FUNCTION                                                \
  auto operator OP (const T1& l, const T2& r)                           \
  {                                                                     \
    using  ret_t = BinaryPredicateExpression<get_expr_node_t<T1>,       \
                                       get_expr_node_t<T2>,             \
                                       BinaryPredicateOp::ENUM>;        \
                                                                        \
    return ret_t(get_expr_node(l),get_expr_node(r));                    \
  }

EKAT_GEN_BIN_PREDICATE_EXPR(==,EQ);
EKAT_GEN_BIN_PREDICATE_EXPR(!=,NE);
EKAT_GEN_BIN_PREDICATE_EXPR(> ,GT);
EKAT_GEN_BIN_PREDICATE_EXPR(>=,GE);
EKAT_GEN_BIN_PREDICATE_EXPR(< ,LT);
EKAT_GEN_BIN_PREDICATE_EXPR(<=,LE);
EKAT_GEN_BIN_PREDICATE_EXPR(&&,AND);
EKAT_GEN_BIN_PREDICATE_EXPR(||,OR);

#undef EKAT_GEN_BIN_PREDICATE_EXPR

} // namespace ekat

#endif // EKAT_EXPRESSION_BINARY_PREDICATE_HPP
