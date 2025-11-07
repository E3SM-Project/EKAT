#ifndef EKAT_VIEW_EXPRESSION_HPP
#define EKAT_VIEW_EXPRESSION_HPP

#include "ekat_expression_meta.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

template<typename ViewT>
class ViewExpression {
public:
  using view_t = ViewT;
  using value_t = typename ViewT::element_type;

  static constexpr bool is_assignable = true;

  ViewExpression (const view_t& v)
   : m_view(v)
  {
    // Nothing to do here
  }

  static constexpr int rank () { return ViewT::rank; }
  int extent (int i) const { return m_view.extent_int(i); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  const value_t& eval(Args... args) const {
    static_assert(sizeof...(Args)==ViewT::rank, "Something is off...\n");
    return m_view(args...);
  }
  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  value_t& access(Args... args) const {
    static_assert(sizeof...(Args)==ViewT::rank, "Something is off...\n");
    return m_view(args...);
  }

protected:

  view_t m_view;
};

// Specialize meta utils
template<typename ViewT>
struct is_expr<ViewExpression<ViewT>> : std::true_type {};

template<typename ViewT>
struct is_assignable_expr<ViewExpression<ViewT>> : std::bool_constant<not std::is_const_v<typename ViewExpression<ViewT>::value_t>> {};

template<typename ViewT>
struct eval_return<ViewExpression<ViewT>> {
  using type = typename ViewExpression<ViewT>::value_t;
};

} // namespace ekat

#endif // EKAT_VIEW_EXPRESSION_HPP
