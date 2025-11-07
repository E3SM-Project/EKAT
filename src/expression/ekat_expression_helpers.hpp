#ifndef EKAT_EXPRESSION_HELPERS_HPP
#define EKAT_EXPRESSION_HELPERS_HPP

#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_compare.hpp"
#include "ekat_expression_conditional.hpp"
#include "ekat_expression_math.hpp"
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

} // namespace ekat

#endif // EKAT_EXPRESSION_HELPERS_HPP
