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

  KOKKOS_INLINE_FUNCTION
  ViewExpression (const view_t& v)
   : m_view(v)
  {
    // Nothing to do here
  }

  static constexpr int rank () { return ViewT::rank; }
  KOKKOS_INLINE_FUNCTION int extent (int i) const { return m_view.extent_int(i); }

  // Elemental: no pre-computation needed
  static constexpr ExprKind kind () { return ExprKind::Elemental; }

  template<typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void setup (const MemberType&) const { /* no-op */ }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  const value_t& eval(Args... args) const {
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
struct eval_return<ViewExpression<ViewT>> {
  using type = typename ViewExpression<ViewT>::value_t;
};

} // namespace ekat

#endif // EKAT_VIEW_EXPRESSION_HPP
