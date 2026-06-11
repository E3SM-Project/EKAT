#ifndef EKAT_VIEW_EXPRESSION_HPP
#define EKAT_VIEW_EXPRESSION_HPP

#include "ekat_expression_base.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

template<typename ViewT>
class ViewExpression : public ExpressionBase<ViewExpression<ViewT>> {
public:
  using view_t = ViewT;
  using return_type = typename ViewT::element_type;

  KOKKOS_INLINE_FUNCTION
  ViewExpression (const view_t& v)
   : m_view(v)
  {
    // Nothing to do here
  }

  static constexpr int rank () { return ViewT::rank; }
  KOKKOS_INLINE_FUNCTION
  int extent (int i) const { return m_view.extent_int(i); }

  template<typename... Args,
           typename = std::enable_if_t<(std::is_integral_v<Args> && ...)>>
  KOKKOS_INLINE_FUNCTION
  auto eval(Args... args) const {
    static_assert(sizeof...(Args)==ViewT::rank, "Something is off...\n");
    return m_view(args...);
  }

  // Even if inside a hyerarchical parallel loop, we simply evaluate the view
  template<typename TeamMember, typename... Args,
           typename = std::enable_if_t<(std::is_integral_v<Args> && ...)>>
  KOKKOS_INLINE_FUNCTION
  auto eval (const TeamEvalSpecs<TeamMember>&, Args... args) const {
    static_assert(sizeof...(Args)==ViewT::rank, "Something is off...\n");
    return m_view(args...);
  }

protected:
  view_t m_view;
};

// Free fcn to construct a ViewExpression
template<typename ViewT,
         typename = std::enable_if_t<Kokkos::is_view_v<ViewT>>>
KOKKOS_INLINE_FUNCTION
auto expression(const ViewT& v)
{
  return ViewExpression<ViewT>(v);
}

} // namespace ekat

#endif // EKAT_VIEW_EXPRESSION_HPP
