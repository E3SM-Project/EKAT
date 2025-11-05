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
  auto eval(Args... args) const {
    if constexpr (scalar_base) {
      return Kokkos::pow(m_base,m_exp.eval(args...));
    } else if constexpr (scalar_exp) {
      return Kokkos::pow(m_base.eval(args...),m_exp);
    } else {
      return Kokkos::pow(m_base.eval(args...),m_exp.eval(args...));
    }
  }

  static auto ret_type () {
    if constexpr (scalar_base) {
      using type = decltype(Kokkos::pow(std::declval<EBase>(),EExp::ret_type()));
      return type(0);
    } else if constexpr (scalar_exp) {
      using type = decltype(Kokkos::pow(EBase::ret_type(),std::declval<EExp>()));
      return type(0);
    } else {
      using type = decltype(Kokkos::pow(EBase::ret_type(),EExp::ret_type()));
      return type(0);
    }
  }
protected:

  EBase  m_base;
  EExp   m_exp;
};

template<typename EBase, typename EExp>
struct is_expr<PowExpression<EBase,EExp>> : std::true_type {};

template<typename EBase, typename EExp>
PowExpression<EBase,EExp>
pow (const Expression<EBase>& b, const Expression<EExp>& e)
{
  return PowExpression<EBase,EExp>(b.cast(),e.cast());
}

// Pow from scalars
template<typename EBase,typename ST>
std::enable_if_t<std::is_arithmetic_v<ST>,PowExpression<EBase,ST>>
pow (const Expression<EBase>& b, const ST e)
{
  return PowExpression<EBase,ST>(b.cast(),e);
}

template<typename ST,typename EExp>
std::enable_if_t<std::is_arithmetic_v<ST>,PowExpression<ST,EExp>>
pow (const ST b, const Expression<EExp>& e)
{
  return PowExpression<ST,EExp>(b,e.cast());
}

// ----------------- Unary math fcns ------------------- //

#define UNARY_MATH_EXPRESSION(impl,name) \
  template<typename EArg>                                               \
  class name##Expression : public Expression<name##Expression<EArg>> {  \
  public:                                                               \
    name##Expression (const EArg& arg)                                  \
      : m_arg(arg)                                                      \
    {}                                                                  \
                                                                        \
    int num_indices () const { return m_arg.num_indices(); }            \
                                                                        \
    template<typename... Args>                                          \
    KOKKOS_INLINE_FUNCTION                                              \
    auto eval(Args... args) const {                                     \
      return Kokkos::impl(m_arg.eval(args...));                         \
    }                                                                   \
    static auto ret_type () {                                           \
      using type = decltype(Kokkos::impl(EArg::ret_type()));            \
      return type(0);                                                   \
    }                                                                   \
  protected:                                                            \
    EArg    m_arg;                                                      \
  };                                                                    \
                                                                        \
  template<typename EArg>                                               \
  name##Expression<EArg>                                                \
  impl (const Expression<EArg>& arg)                                    \
  {                                                                     \
    return name##Expression<EArg>(arg.cast());                          \
  }                                                                     \
                                                                        \
  template<typename EArg>                                               \
  struct is_expr<name##Expression<EArg>> : std::true_type {};

UNARY_MATH_EXPRESSION (sqrt,Sqrt)
UNARY_MATH_EXPRESSION (exp,Exp)
UNARY_MATH_EXPRESSION (log,Log)
UNARY_MATH_EXPRESSION (sin,Sin)
UNARY_MATH_EXPRESSION (cos,Cos)
#undef UNARY_MATH_EXPRESSION

} // namespace ekat

#endif // EKAT_EXPRESSION_MATH_HPP
