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

  KOKKOS_INLINE_FUNCTION
  Real eval(int i) const {
    return cast().eval(i);
  }

  KOKKOS_INLINE_FUNCTION
  Real eval(int i, int j) const {
    return cast().eval(i,j);
  }

  KOKKOS_INLINE_FUNCTION
  Real eval(int i, int j, int k) const {
    return cast().eval(i,j,k);
  }

  KOKKOS_INLINE_FUNCTION
  const Derived& cast () const { return static_cast<const Derived&>(*this); }
        Derived& cast ()       { return static_cast<      Derived&>(*this); }
};

} // namespace ekat

#endif // EKAT_EXPRESSION_HPP
