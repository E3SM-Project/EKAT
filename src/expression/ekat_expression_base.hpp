#ifndef EKAT_EXPRESSION_HPP
#define EKAT_EXPRESSION_HPP

#include <Kokkos_Core.hpp>

namespace ekat {

template<typename Derived>
class Expression {
public:

  static constexpr int rank () { return Derived::rank(); }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  auto eval(Args... args) const {
    static_assert(std::conjunction_v<std::is_integral<Args>...>,
                  "[Expression::eval] Error! All arguments must be integral types!");
    static_assert(sizeof...(Args) <= 7,
                  "[Expression::eval] Error! The number of arguments must be between 0 and 7.");
    return cast().eval(args...);
  }

  KOKKOS_INLINE_FUNCTION
  const Derived& cast () const { return static_cast<const Derived&>(*this); }
};

// Some meta-utilities that will prove useful in derived classes

// Detect if a type is an Expression
template<typename T>
struct is_expr : std::false_type {};
template<typename D>
struct is_expr<Expression<D>> : std::true_type {};
template<typename T>
constexpr bool is_expr_v = is_expr<T>::value;

// Deduce the type obtained evaluating an input type T
template<typename T, bool is_expr>
struct EvaluationType {
  using type = T;
};
template<typename T>
struct EvaluationType<T,true> {
  using type = decltype(T::ret_type());
};
template<typename T>
using eval_t = typename EvaluationType<T,is_expr_v<T>>::type;

} // namespace ekat

#endif // EKAT_EXPRESSION_HPP
