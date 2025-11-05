#ifndef EKAT_EXPRESSION_HELPERS_HPP
#define EKAT_EXPRESSION_HELPERS_HPP

#include "ekat_expression_base.hpp"
#include "ekat_expression_view.hpp"
#include "ekat_expression_compare.hpp"
#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_math.hpp"

namespace ekat {

// Support the case where LHS and/or RHS of a C++ operator is a View

// -------------- Unary minus -------------//

template<typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,
                 BinaryExpression<int,ViewExpression<ViewT>,BinOp::Mult>>
operator-(const ViewT& v)
{
  return -1*view_expression(v);
}

// ---------------- Plus ----------------- //

// view+expression
template<typename ERight,typename ViewT>
auto operator+ (const ViewT& left, const Expression<ERight>& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ERight,BinOp::Plus>>
{
  return view_expression(left)+right;
}

// expression+view
template<typename ELeft,typename ViewT>
auto operator+ (const Expression<ELeft>& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ELeft,ViewExpression<ViewT>,BinOp::Plus>>
{
  return left+view_expression(right);
}

// view+view
template<typename ViewT>
auto operator+ (const ViewT& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ViewExpression<ViewT>,BinOp::Plus>>
{
  return view_expression(left)+view_expression(right);
}

// scalar+view
template<typename ST,typename ViewT>
auto operator+ (const ST& left, const ViewT& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ST,ViewExpression<ViewT>,BinOp::Plus>>
{
  return left+view_expression(right);
}

// view+scalar
template<typename ViewT,typename ST>
auto operator+ (const ViewT& left, const ST& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ST,BinOp::Plus>>
{
  return view_expression(left)+right;
}

// ---------------- Minus ----------------- //

// view-expression
template<typename ERight,typename ViewT>
auto operator- (const ViewT& left, const Expression<ERight>& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ERight,BinOp::Minus>>
{
  return view_expression(left)-right;
}

// expression-view
template<typename ELeft,typename ViewT>
auto operator- (const Expression<ELeft>& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ELeft,ViewExpression<ViewT>,BinOp::Minus>>
{
  return left-view_expression(right);
}

// view-view
template<typename ViewT>
auto operator- (const ViewT& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ViewExpression<ViewT>,BinOp::Minus>>
{
  return view_expression(left)-view_expression(right);
}

// scalar-view
template<typename ST,typename ViewT>
auto operator- (const ST& left, const ViewT& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ST,ViewExpression<ViewT>,BinOp::Minus>>
{
  return left-view_expression(right);
}

// view-scalar
template<typename ViewT,typename ST>
auto operator- (const ViewT& left, const ST& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ST,BinOp::Minus>>
{
  return view_expression(left)-right;
}

// ---------------- Mult ----------------- //

// view*expression
template<typename ERight,typename ViewT>
auto operator* (const ViewT& left, const Expression<ERight>& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ERight,BinOp::Mult>>
{
  return view_expression(left)*right;
}

// expression*view
template<typename ELeft,typename ViewT>
auto operator* (const Expression<ELeft>& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ELeft,ViewExpression<ViewT>,BinOp::Mult>>
{
  return left*view_expression(right);
}

// view*view
template<typename ViewT>
auto operator* (const ViewT& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ViewExpression<ViewT>,BinOp::Mult>>
{
  return view_expression(left)*view_expression(right);
}

// scalar*view
template<typename ST,typename ViewT>
auto operator* (const ST& left, const ViewT& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ST,ViewExpression<ViewT>,BinOp::Mult>>
{
  return left*view_expression(right);
}

// view*scalar
template<typename ViewT,typename ST>
auto operator* (const ViewT& left, const ST& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ST,BinOp::Mult>>
{
  return view_expression(left)*right;
}

// ---------------- Div ----------------- //

// view/expression
template<typename ERight,typename ViewT>
auto operator/ (const ViewT& left, const Expression<ERight>& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ERight,BinOp::Div>>
{
  return view_expression(left)/right;
}

// expression/view
template<typename ELeft,typename ViewT>
auto operator/ (const Expression<ELeft>& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ELeft,ViewExpression<ViewT>,BinOp::Div>>
{
  return left/view_expression(right);
}

// view/view
template<typename ViewT>
auto operator/ (const ViewT& left, const ViewT& right)
 -> std::enable_if_t<Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ViewExpression<ViewT>,BinOp::Div>>
{
  return view_expression(left)/view_expression(right);
}

// scalar/view
template<typename ST,typename ViewT>
auto operator/ (const ST& left, const ViewT& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ST,ViewExpression<ViewT>,BinOp::Div>>
{
  return left/view_expression(right);
}

// view/scalar
template<typename ViewT,typename ST>
auto operator/ (const ViewT& left, const ST& right)
 -> std::enable_if_t<std::is_arithmetic_v<ST> and Kokkos::is_view_v<ViewT>,
                     BinaryExpression<ViewExpression<ViewT>,ST,BinOp::Div>>
{
  return view_expression(left)/right;
}

// Pow from view
template<typename ViewT, typename EExp>
std::enable_if_t<Kokkos::is_view_v<ViewT>,
                 PowExpression<ViewExpression<ViewT>,EExp>>
pow (const ViewT& b, const EExp& e)
{
  return PowExpression<ViewExpression<ViewT>,EExp>(view_expression(b),e);
}

template<typename EBase,typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,
                 PowExpression<EBase,ViewExpression<ViewT>>>
pow (const EBase& b, const ViewT& e)
{
  return PowExpression<EBase,ViewExpression<ViewT>>(b,view_expression(e));
}

// Pow from scalars
template<typename EBase,typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,PowExpression<EBase,ST>>
pow (const Expression<EBase>& b, const ST e)
{
  return PowExpression<EBase,ST>(b.cast(),e);
}

template<typename ST,typename EExp>
std::enable_if_t<std::is_arithmetic_v<ST>,PowExpression<ST,EExp>>
pow (const ST b, const Expression<EExp>& e)
{
  return PowExpression<ST,EExp>(b,e.cast());
}

// ------------------- Unary math fcns --------------------- //

#define UNARY_MATH_EXPRESSION_FROM_FIELD(impl,name)           \
  template<typename ViewT>                                    \
  std::enable_if_t<Kokkos::is_view_v<ViewT>,                  \
                   name##Expression<ViewExpression<ViewT>>>   \
  impl (const ViewT& v)                                       \
  {                                                           \
    using VE = ViewExpression<ViewT>;                         \
    return name##Expression<VE>(VE(v));                       \
  }

UNARY_MATH_EXPRESSION_FROM_FIELD (sqrt,Sqrt)
UNARY_MATH_EXPRESSION_FROM_FIELD (exp,Exp)
UNARY_MATH_EXPRESSION_FROM_FIELD (log,Log)
UNARY_MATH_EXPRESSION_FROM_FIELD (sin,Sin)
UNARY_MATH_EXPRESSION_FROM_FIELD (cos,Cos)

#undef UNARY_MATH_EXPRESSION_FROM_FIELD_OR_SCALAR

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
