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

  ViewExpression (const view_t& v)
   : m_view(v)
  {
    // Nothing to do here
  }

  static constexpr int rank () { return ViewT::rank; }
  int extent (int i) const { return m_view.extent_int(i); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  const return_type& eval(Args... args) const {
    static_assert(sizeof...(Args)==ViewT::rank, "Something is off...\n");
    return m_view(args...);
  }

protected:
  view_t m_view;
};

// Free fcn to construct a ViewExpression
template<typename ViewT,
         typename = std::enable_if_t<Kokkos::is_view_v<ViewT>>>
auto expression(const ViewT& v)
{
  return ViewExpression<ViewT>(v);
}

} // namespace ekat

#endif // EKAT_VIEW_EXPRESSION_HPP
