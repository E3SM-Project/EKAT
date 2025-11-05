#ifndef EKAT_VIEW_EXPRESSION_HPP
#define EKAT_VIEW_EXPRESSION_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

// TODO: support 4+ dim. Also 0d?
template<typename ViewT>
class ViewExpression : public Expression<ViewExpression<ViewT>> {
public:
  using view_t = ViewT;
  static_assert(view_t::rank >=1 and view_t::rank <=3,
      "Unsupported rank for ViewExpression");

  ViewExpression (const view_t& v)
   : m_view(v)
  {
    // Nothing to do here
  }

  int num_indices () const { return ViewT::rank; }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  Real eval(Args... args) const {
    return m_view(args...);
  }

protected:

  view_t m_view;
};

template<typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,ViewExpression<ViewT>>
view_expression(const ViewT& v)
{
  return ViewExpression<ViewT>(v);
}

} // namespace ekat

#endif // EKAT_VIEW_EXPRESSION_HPP
