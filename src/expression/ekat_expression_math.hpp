#ifndef EKAT_EXPRESSION_MATH_HPP
#define EKAT_EXPRESSION_MATH_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

// ----------------- POW ------------------- //

template<typename EBase, typename EExp>
class PowExpression : public Expression<PowExpression<EBase,EExp>> {
public:
  static constexpr bool scalar_base = std::is_arithmetic_v<EBase>;
  static constexpr bool scalar_exp  = std::is_arithmetic_v<EExp>;
  static_assert(not scalar_base or not scalar_exp,
      "[PowExpression] One between EBase and EExp must be non-arithmetic.\n");

  PowExpression (const EBase& base, const EExp& exp)
    : m_base(base)
    , m_exp(exp)
  {
    // Nothing to do here
  }

  int num_indices () const {
    if constexpr (scalar_base) {
      return m_exp.num_indices();
    } else if constexpr (scalar_exp) {
      return m_base.num_indices();
    } else {
      return std::max(m_base.num_indices(),m_exp.num_indices());
    }
  }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  Real eval(Args... args) const {
    if constexpr (scalar_base) {
      return Kokkos::pow(m_base,m_exp.eval(args...));
    } else if constexpr (scalar_exp) {
      return Kokkos::pow(m_base.eval(args...),m_exp);
    } else {
      return Kokkos::pow(m_base.eval(args...),m_exp.eval(args...));
    }
  }
protected:

  EBase  m_base;
  EExp   m_exp;
};

template<typename EBase, typename EExp>
PowExpression<EBase,EExp>
pow (const Expression<EBase>& b, const Expression<EExp>& e)
{
  return PowExpression<EBase,EExp>(b.cast(),e.cast());
}

// ----------------- Unary math fcns ------------------- //

#define UNARY_MATH_EXPRESSION(impl,name) \
  template<typename EArg>                                               \
  class name##Expression : public Expression<name##Expression<EArg>> {  \
  public:                                                               \
    name##Expression (const EArg& arg)                                  \
      : m_arg(arg)                                                      \
    {                                                                   \
      /* Nothing to do here */                                          \
    }                                                                   \
                                                                        \
    int num_indices () const { return m_arg.num_indices(); }            \
                                                                        \
    template<typename... Args>                                          \
    KOKKOS_INLINE_FUNCTION                                              \
    Real eval(Args... args) const {                                     \
      return Kokkos::impl(m_arg.eval(args...));                         \
    }                                                                   \
  protected:                                                            \
                                                                        \
    EArg    m_arg;                                                      \
  };                                                                    \
                                                                        \
  template<typename EArg>                                               \
  name##Expression<EArg>                                                \
  impl (const Expression<EArg>& arg)                                    \
  {                                                                     \
    return name##Expression<EArg>(arg.cast());                          \
  }

UNARY_MATH_EXPRESSION (sqrt,Sqrt)
UNARY_MATH_EXPRESSION (exp,Exp)
UNARY_MATH_EXPRESSION (log,Log)
UNARY_MATH_EXPRESSION (sin,Sin)
UNARY_MATH_EXPRESSION (cos,Cos)
#undef UNARY_MATH_EXPRESSION

} // namespace ekat

#endif // EKAT_EXPRESSION_MATH_HPP
