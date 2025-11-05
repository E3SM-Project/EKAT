#ifndef EKAT_EXPRESSION_HELPERS_HPP
#define EKAT_EXPRESSION_HELPERS_HPP

#include "ekat_expression_base.hpp"
#include "ekat_expression_view.hpp"
#include "ekat_expression_compare.hpp"
#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_math.hpp"

namespace ekat {

// --------------- expression CMP scalar --------------- //

template<typename ELeft, typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ELeft,ST>>
operator== (const Expression<ELeft>& l, const ST r)
{
  return cmp_expression(l.cast(),r,Comparison::EQ);
}
template<typename ELeft, typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ELeft,ST>>
operator!= (const Expression<ELeft>& l, const ST r)
{
  return cmp_expression(l.cast(),r,Comparison::NE);
}
template<typename ELeft, typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ELeft,ST>>
operator> (const Expression<ELeft>& l, const ST r)
{
  return cmp_expression(l.cast(),r,Comparison::GT);
}
template<typename ELeft, typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ELeft,ST>>
operator>= (const Expression<ELeft>& l, const ST r)
{
  return cmp_expression(l.cast(),r,Comparison::GE);
}
template<typename ELeft, typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ELeft,ST>>
operator< (const Expression<ELeft>& l, const ST r)
{
  return cmp_expression(l.cast(),r,Comparison::LT);
}
template<typename ELeft, typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ELeft,ST>>
operator<= (const Expression<ELeft>& l, const ST r)
{
  return cmp_expression(l.cast(),r,Comparison::LE);
}

// --------------- scalar CMP expression --------------- //

template<typename ST, typename ERight>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ST,ERight>>
operator== (const ST l, const Expression<ERight>& r)
{
  return cmp_expression(l,r.cast(),Comparison::EQ);
}
template<typename ST, typename ERight>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ST,ERight>>
operator!= (const ST l, const Expression<ERight>& r)
{
  return cmp_expression(l,r.cast(),Comparison::NE);
}
template<typename ST, typename ERight>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ST,ERight>>
operator> (const ST l, const Expression<ERight>& r)
{
  return cmp_expression(l,r.cast(),Comparison::GT);
}
template<typename ST, typename ERight>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ST,ERight>>
operator>= (const ST l, const Expression<ERight>& r)
{
  return cmp_expression(l,r.cast(),Comparison::GE);
}
template<typename ST, typename ERight>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ST,ERight>>
operator< (const ST l, const Expression<ERight>& r)
{
  return cmp_expression(l,r.cast(),Comparison::LT);
}
template<typename ST, typename ERight>
std::enable_if_t<std::is_arithmetic_v<ST>,CmpExpression<ST,ERight>>
operator<= (const ST l, const Expression<ERight>& r)
{
  return cmp_expression(l,r.cast(),Comparison::LE);
}

// --------------- view CMP something --------------- //

template<typename ViewT, typename ERight>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ViewExpression<ViewT>,ERight>>
operator== (const ViewT& l, const ERight& r)
{
  return cmp_expression(l,view_expression(r),Comparison::EQ);
}
template<typename ViewT, typename ERight>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ViewExpression<ViewT>,ERight>>
operator!= (const ViewT& l, const ERight& r)
{
  return cmp_expression(l,view_expression(r),Comparison::NE);
}
template<typename ViewT, typename ERight>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ViewExpression<ViewT>,ERight>>
operator> (const ViewT& l, const ERight& r)
{
  return cmp_expression(l,view_expression(r),Comparison::GT);
}
template<typename ViewT, typename ERight>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ViewExpression<ViewT>,ERight>>
operator>= (const ViewT& l, const ERight& r)
{
  return cmp_expression(l,view_expression(r),Comparison::GE);
}
template<typename ViewT, typename ERight>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ViewExpression<ViewT>,ERight>>
operator< (const ViewT& l, const ERight& r)
{
  return cmp_expression(l,view_expression(r),Comparison::LT);
}
template<typename ViewT, typename ERight>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ViewExpression<ViewT>,ERight>>
operator<= (const ViewT& l, const ERight& r)
{
  return cmp_expression(l,view_expression(r),Comparison::LE);
}
// --------------- something CMP view --------------- //

template<typename ELeft, typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ELeft,ViewExpression<ViewT>>>
operator== (const ELeft& l, const ViewT& r)
{
  return cmp_expression(view_expression(l),r,Comparison::EQ);
}
template<typename ELeft, typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ELeft,ViewExpression<ViewT>>>
operator!= (const ELeft& l, const ViewT& r)
{
  return cmp_expression(view_expression(l),r,Comparison::NE);
}
template<typename ELeft, typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ELeft,ViewExpression<ViewT>>>
operator> (const ELeft& l, const ViewT& r)
{
  return cmp_expression(view_expression(l),r,Comparison::GT);
}
template<typename ELeft, typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ELeft,ViewExpression<ViewT>>>
operator>= (const ELeft& l, const ViewT& r)
{
  return cmp_expression(view_expression(l),r,Comparison::GE);
}
template<typename ELeft, typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ELeft,ViewExpression<ViewT>>>
operator< (const ELeft& l, const ViewT& r)
{
  return cmp_expression(view_expression(l),r,Comparison::LT);
}
template<typename ELeft, typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,CmpExpression<ELeft,ViewExpression<ViewT>>>
operator<= (const ELeft& l, const ViewT& r)
{
  return cmp_expression(view_expression(l),r,Comparison::LE);
}


} // namespace ekat

#endif // EKAT_EXPRESSION_HELPERS_HPP
