#ifndef EKAT_VIEW_EXPRESSION_HPP
#define EKAT_VIEW_EXPRESSION_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

template<typename ViewT>
class ViewExpression : public Expression<ViewExpression<ViewT>> {
public:
  using view_t = ViewT;
  using value_t = typename ViewT::element_type;

  ViewExpression (const view_t& v)
   : m_view(v)
  {
    // Nothing to do here
  }

  int num_indices () const { return ViewT::rank; }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  value_t eval(Args... args) const {
    static_assert(sizeof...(Args)==ViewT::rank, "Something is off...\n");
    return m_view(args...);
  }

  static value_t ret_type () { return 0; }

protected:

  view_t m_view;
};

template<typename ViewT>
struct is_expr<ViewExpression<ViewT>> : std::true_type {};

template<typename ViewT>
std::enable_if_t<Kokkos::is_view_v<ViewT>,ViewExpression<ViewT>>
view_expression(const ViewT& v)
{
  return ViewExpression<ViewT>(v);
}

} // namespace ekat

#endif // EKAT_VIEW_EXPRESSION_HPP
