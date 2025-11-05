#ifndef EKAT_EXPRESSION_HPP
#define EKAT_EXPRESSION_HPP

#include <Kokkos_Core.hpp>

namespace ekat {

// For now. Later, template on return type maybe?
using Real = double;

template<typename Derived>
class Expression {
public:

  int num_indices () const { return cast().num_indices(); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  Real eval(Args... args) const {
    static_assert(std::conjunction_v<std::is_integral<Args>...>,
                  "[Expression] All arguments must be integral types!");
    static_assert(sizeof...(Args) <= 7,
                  "[Expression] The number of arguments must be between 0 and 7.");
    return cast().eval(args...);
  }

  KOKKOS_INLINE_FUNCTION
  const Derived& cast () const { return static_cast<const Derived&>(*this); }
        Derived& cast ()       { return static_cast<      Derived&>(*this); }
};

} // namespace ekat

#endif // EKAT_EXPRESSION_HPP
