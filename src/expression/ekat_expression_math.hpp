#ifndef EKAT_EXPRESSION_MATH_HPP
#define EKAT_EXPRESSION_MATH_HPP

#include "ekat_expression_meta.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

// ----------------- POW ------------------- //

template<typename EBase, typename EExp>
class PowExpression {
public:
  static constexpr bool expr_b = is_expr_v<EBase>;
  static constexpr bool expr_e = is_expr_v<EExp>;

  using eval_base_t = eval_return_t<EBase>;
  using eval_exp_t  = eval_return_t<EExp>;
  using eval_t      = decltype(Kokkos::pow(std::declval<eval_base_t>(),std::declval<eval_exp_t>()));

  // Don't create an expression from builtin types, just call pow!
  static_assert(expr_b or expr_e,
      "[PowExpression] Error! One between EBase and EExp must be an Expression type.\n");

  PowExpression (const EBase& base, const EExp& exp)
    : m_base(base)
    , m_exp(exp)
  {
    // Nothing to do here
  }

  static constexpr int rank() {
    if constexpr (not expr_b) {
      return EExp::rank();
    } else if constexpr (not expr_e) {
      return EBase::rank();
    } else {
      static_assert(EBase::rank()==EExp::rank(),
        "[PowExpression] Error! EBase and EExp are Expression types of different rank.\n");
      return EBase::rank();
    }
  }
  int extent (int i) const {
    if constexpr (expr_b)
      return m_base.extent(i);
    else
      return m_exp.extent(i);
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  auto eval(Args... args) const {
    if constexpr (not expr_b) {
      return Kokkos::pow(m_base,m_exp.eval(args...));
    } else if constexpr (not expr_e) {
      return Kokkos::pow(m_base.eval(args...),m_exp);
    } else {
      return Kokkos::pow(m_base.eval(args...),m_exp.eval(args...));
    }
  }

protected:

  EBase  m_base;
  EExp   m_exp;
};

// Specialize meta utils
template<typename EBase, typename EExp>
struct is_expr<PowExpression<EBase,EExp>> : std::true_type {};
template<typename EBase, typename EExp>
struct eval_return<PowExpression<EBase,EExp>> {
  using type = typename PowExpression<EBase,EExp>::eval_t;
};

// Free fcn to construct a PowExpression
template<typename EBase, typename EExp>
std::enable_if_t<is_expr_v<EBase> or is_expr_v<EExp>,PowExpression<EBase,EExp>>
pow (const EBase& b, const EExp& e)
{
  return PowExpression<EBase,EExp>(b,e);
}

// ----------------- Unary math fcns ------------------- //

#define UNARY_MATH_EXPRESSION(impl,name) \
  template<typename EArg>                                               \
  class name##Expression {                                              \
  public:                                                               \
    using arg_eval_t = eval_return_t<EArg>;                             \
    using eval_t = decltype(Kokkos::impl(std::declval<arg_eval_t>()));  \
                                                                        \
    name##Expression (const EArg& arg)                                  \
      : m_arg(arg)                                                      \
    {}                                                                  \
                                                                        \
    static constexpr int rank() { return EArg::rank(); }                \
    int extent (int i) const { return m_arg.extent(i); }                \
                                                                        \
    template<typename... Args>                                          \
    KOKKOS_INLINE_FUNCTION                                              \
    eval_t eval(Args... args) const {                                   \
      return Kokkos::impl(m_arg.eval(args...));                         \
    }                                                                   \
  protected:                                                            \
    EArg    m_arg;                                                      \
  };                                                                    \
                                                                        \
  /* Free function to create a ##nameExpression */                      \
  template<typename EArg>                                               \
  std::enable_if_t<is_expr_v<EArg>,name##Expression<EArg>>              \
  impl (const EArg& arg)                                                \
  {                                                                     \
    return name##Expression<EArg>(arg);                                 \
  }                                                                     \
                                                                        \
  /* Specialize meta util */                                            \
  template<typename EArg>                                               \
  struct is_expr<name##Expression<EArg>> : std::true_type {};           \
  template<typename EArg>                                               \
  struct eval_return<name##Expression<EArg>> {                          \
    using type = typename name##Expression<EArg>::eval_t;               \
  };

UNARY_MATH_EXPRESSION (sqrt,Sqrt)
UNARY_MATH_EXPRESSION (exp,Exp)
UNARY_MATH_EXPRESSION (log,Log)
UNARY_MATH_EXPRESSION (sin,Sin)
UNARY_MATH_EXPRESSION (cos,Cos)
#undef UNARY_MATH_EXPRESSION

} // namespace ekat

#endif // EKAT_EXPRESSION_MATH_HPP
