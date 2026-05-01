#ifndef EKAT_EXPRESSION_BASE_HPP
#define EKAT_EXPRESSION_BASE_HPP

#include "ekat_expression_traits.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

// A base class for expressions
template<typename Derived>
class ExpressionBase {
public:
  using expression_tag = void; // Add tag to be used for SFINAE and meta-utils

  ExpressionBase () {
    static_assert(is_expr_v<Derived>,
        "Template arg is NOT an expression. Ensure Derived inherits from ExpressionBase.");
  }

  Derived& cast () { return *static_cast<Derived*>(this); }
  const Derived& cast () const { return *static_cast<const Derived*>(this); }

  ExpressionBase<Derived>& as_base () { return *this; }
  const ExpressionBase<Derived>& as_base () const { return *this; }

  static constexpr int rank() { return Derived::rank(); }

  int extent (int i) const { return cast().extent(i); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  auto eval (Args... args) const
  {
    return cast().eval(args...);
  }
};

} // namespace ekat

#endif // EKAT_EXPRESSION_BASE_HPP
