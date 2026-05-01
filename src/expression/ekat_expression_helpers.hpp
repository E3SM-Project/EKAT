#ifndef EKAT_EXPRESSION_HELPERS_HPP
#define EKAT_EXPRESSION_HELPERS_HPP

#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_compare.hpp"
#include "ekat_expression_conditional.hpp"
#include "ekat_expression_math.hpp"
#include "ekat_expression_reduce.hpp"
#include "ekat_expression_view.hpp"

namespace ekat {
// Unary minus implemented as -1*expr
template<typename ERight>
std::enable_if_t<is_expr_v<ERight>,BinaryExpression<int,ERight,BinOp::Mult>>
operator- (const ERight& r)
{
  return BinaryExpression<int,ERight,BinOp::Mult>(-1,r);
}

// Overload arithmetic operators
template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Plus>>
operator+ (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Plus>(l,r);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Minus>>
operator- (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Minus>(l,r);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Mult>>
operator* (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Mult>(l,r);
}

template<typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ELeft> or is_expr_v<ERight>,BinaryExpression<ELeft,ERight,BinOp::Div>>
operator/ (const ELeft& l, const ERight& r)
{
  return BinaryExpression<ELeft,ERight,BinOp::Div>(l,r);
}

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

// Free fcn to construct a ConditionalExpression
template<typename ECond, typename ELeft, typename ERight>
std::enable_if_t<is_expr_v<ECond> or is_expr_v<ELeft> or is_expr_v<ERight>,ConditionalExpression<ECond,ELeft,ERight>>
conditional(const ECond& c, const ELeft& l, const ERight& r)
{
  return ConditionalExpression<ECond,ELeft,ERight>(c,l,r);
}

// Free fcn to construct a ViewExpression
template<typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,ViewExpression<ViewT>>
view_expression(const ViewT& v)
{
  return ViewExpression<ViewT>(v);
}

// -----------------------------------------------------------------------
// Free functions to build ReduceExpression nodes
//
// The 'result' view must be pre-allocated on the host with at least
// league_size elements (one per team).  All helpers require the
// sub-expression to be rank-1.
// -----------------------------------------------------------------------

template<typename EArg>
std::enable_if_t<is_expr_v<EArg>,
                 ReduceExpression<EArg,ReduceOp::Sum>>
reduce_sum (const EArg& sub,
            const Kokkos::View<eval_return_t<EArg>*>& result)
{
  return ReduceExpression<EArg,ReduceOp::Sum>(sub, result);
}

template<typename EArg>
std::enable_if_t<is_expr_v<EArg>,
                 ReduceExpression<EArg,ReduceOp::Max>>
reduce_max (const EArg& sub,
            const Kokkos::View<eval_return_t<EArg>*>& result)
{
  return ReduceExpression<EArg,ReduceOp::Max>(sub, result);
}

template<typename EArg>
std::enable_if_t<is_expr_v<EArg>,
                 ReduceExpression<EArg,ReduceOp::Min>>
reduce_min (const EArg& sub,
            const Kokkos::View<eval_return_t<EArg>*>& result)
{
  return ReduceExpression<EArg,ReduceOp::Min>(sub, result);
}

// reduce_all: true if every element satisfies a boolean expression
template<typename EArg>
std::enable_if_t<is_expr_v<EArg>,
                 ReduceExpression<EArg,ReduceOp::And>>
reduce_all (const EArg& sub,
            const Kokkos::View<eval_return_t<EArg>*>& result)
{
  return ReduceExpression<EArg,ReduceOp::And>(sub, result);
}

// reduce_any: true if at least one element satisfies a boolean expression
template<typename EArg>
std::enable_if_t<is_expr_v<EArg>,
                 ReduceExpression<EArg,ReduceOp::Or>>
reduce_any (const EArg& sub,
            const Kokkos::View<eval_return_t<EArg>*>& result)
{
  return ReduceExpression<EArg,ReduceOp::Or>(sub, result);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_HELPERS_HPP
