#ifndef EKAT_EXPRESSION_CONTRACTION__HPP
#define EKAT_EXPRESSION_CONTRACTION__HPP

#include "ekat_expression_base.hpp"

namespace ekat {

enum class ContractionOp : int {
  Sum,
  Prod,
  Avg
};


template<typename EArg, ContractionOp Op>
class ContractionExpression : public ExpressionBase<ContractionExpression<EArg,Op>> {
public:
  static constexpr bool expr_l = is_expr_v<EArg>;

  using return_type = eval_return_t<EArg>;

  // The inner arg MUST be an expression. Its rank must be 1 or 2 (for now)
  static_assert(is_expr_v<EArg> and (EArg::rank()==1 or EArg::rank()==2),
    "[ContractionExpression] EArg MUST be an expression of rank 1 or 2.\n");

  ContractionExpression (const EArg& arg, const int dim_idx)
    : m_arg(left)
    , m_dim_idx(dim_idx)
  {
    // Nothing to do here
  }

  static constexpr int rank() {
    return EArg::rank() - 1;
  }

  int extent (int i) const {
    if constexpr (expr_l)
      return m_arg.extent(i);
    else
      return m_right.extent(i);
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  return_type eval(Args... args) const {
    if constexpr (expr_l) {
      if constexpr (expr_r)
        return eval_impl(m_arg.eval(args...), m_right.eval(args...));
      else
        return eval_impl(m_arg.eval(args...), m_right);
    } else if constexpr (expr_r) {
      return eval_impl(m_arg, m_right.eval(args...));
    } else {
      return eval_impl(m_arg, m_right);
    }
  }

protected:

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  return_type eval_impl(const return_left_t& l, const return_right_t& r) const {
    if constexpr (Op==ContractionOp::EQ)
      return l==r;
    else if constexpr (Op==ContractionOp::NE)
      return l!=r;
    else if constexpr (Op==ContractionOp::GT)
      return l>r;
    else if constexpr (Op==ContractionOp::GE)
      return l>=r;
    else if constexpr (Op==ContractionOp::LT)
      return l<r;
    else if constexpr (Op==ContractionOp::LE)
      return l<=r;
    else if constexpr (Op==ContractionOp::AND)
      return l and r;
    else
      return l or r;
  }

  EArg    m_arg;
  int     m_dim_idx;
};

} // namespace ekat

#endif // EKAT_EXPRESSION_CONTRACTION__HPP
