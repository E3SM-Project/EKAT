#ifndef EKAT_EXPRESSION_MATH_HPP
#define EKAT_EXPRESSION_MATH_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

// ----------------- Binary math fcns ------------------- //

#define EKAT_BINARY_MATH_EXPRESSION(impl,name) \
  template<typename EArg1, typename EArg2>                                          \
  class name##Expression : public ExpressionBase<name##Expression<EArg1,EArg2>>     \
  {                                                                                 \
  public:                                                                           \
    static constexpr bool expr_l = is_expr_v<EArg1>;                                \
    static constexpr bool expr_r = is_expr_v<EArg2>;                                \
                                                                                    \
    using return_arg1_t = eval_return_t<EArg1>;                                     \
    using return_arg2_t = eval_return_t<EArg2>;                                     \
    using return_type = std::common_type_t<return_arg1_t,return_arg2_t>;            \
                                                                                    \
    name##Expression (const EArg1& arg1, const EArg2& arg2)                         \
      : m_arg1(arg1)                                                                \
      , m_arg2(arg2)                                                                \
    {}                                                                              \
                                                                                    \
    static constexpr int rank () {                                                  \
      if constexpr (expr_l) {                                                       \
        if constexpr (expr_r) {                                                     \
          static_assert(EArg1::rank()==EArg2::rank(),                               \
            "[" #name "Expression] Error! EArg1 and EArg2 have different rank.\n"); \
        }                                                                           \
        return EArg1::rank();                                                       \
      } else {                                                                      \
        return EArg2::rank();                                                       \
      }                                                                             \
    }                                                                               \
    int extent (int i) const {                                                      \
      if constexpr (expr_l)                                                         \
        return m_arg1.extent(i);                                                    \
      else                                                                          \
        return m_arg2.extent(i);                                                    \
    }                                                                               \
                                                                                    \
    template<typename... Args>                                                      \
    KOKKOS_INLINE_FUNCTION                                                          \
    return_type eval(Args... args) const {                                          \
      if constexpr (not expr_l)                                                     \
        return eval_impl(m_arg1,m_arg2.eval(args...));                              \
      else if constexpr (not expr_r)                                                \
        return eval_impl(m_arg1.eval(args...),m_arg2);                              \
      else                                                                          \
        return eval_impl(m_arg1.eval(args...),m_arg2.eval(args...));                \
    }                                                                               \
  protected:                                                                        \
    KOKKOS_INLINE_FUNCTION                                                          \
    return_type eval_impl(const return_arg1_t& arg1,                                \
                          const return_arg2_t& arg2) const {                        \
      return Kokkos::impl(static_cast<const return_type&>(arg1),                    \
                          static_cast<const return_type&>(arg2));                   \
    }                                                                               \
                                                                                    \
    EArg1   m_arg1;                                                                 \
    EArg2   m_arg2;                                                                 \
  };                                                                                \
                                                                                    \
  /* Free function to create a ##nameExpression */                                  \
  template<typename T1, typename T2,                                                \
           typename = std::enable_if_t<is_any_expr_v<T1, T2>>>                      \
  auto impl (const T1& arg1, const T2& arg2)                                        \
  {                                                                                 \
    using  ret_t = name##Expression<get_expr_node_t<T1>,                            \
                                    get_expr_node_t<T2>>;                           \
    return ret_t(get_expr_node(arg1),get_expr_node(arg2));                          \
  }

EKAT_BINARY_MATH_EXPRESSION(pow,Pow);
EKAT_BINARY_MATH_EXPRESSION(max,Max);
EKAT_BINARY_MATH_EXPRESSION(min,Min);

#undef EKAT_BINARY_MATH_EXPRESSION

// ----------------- Unary math fcns ------------------- //

#define EKAT_UNARY_MATH_EXPRESSION(impl,name) \
  template<typename EArg>                                                           \
  class name##Expression : public ExpressionBase<name##Expression<EArg>>            \
  {                                                                                 \
  public:                                                                           \
    using return_arg_type = eval_return_t<EArg>;                                    \
    using return_type = decltype(Kokkos::impl(std::declval<return_arg_type>()));    \
                                                                                    \
    name##Expression (const EArg& arg)                                              \
      : m_arg(arg)                                                                  \
    {}                                                                              \
                                                                                    \
    static constexpr int rank() { return EArg::rank(); }                            \
    int extent (int i) const { return m_arg.extent(i); }                            \
                                                                                    \
    template<typename... Args>                                                      \
    KOKKOS_INLINE_FUNCTION                                                          \
    return_type eval(Args... args) const {                                          \
      return Kokkos::impl(m_arg.eval(args...));                                     \
    }                                                                               \
  protected:                                                                        \
    EArg    m_arg;                                                                  \
  };                                                                                \
                                                                                    \
  /* Free function to create a ##nameExpression */                                  \
  template<typename EArg>                                                           \
  std::enable_if_t<is_expr_v<EArg>,                                                 \
                   name##Expression<EArg>>                                          \
  impl (const EArg& arg)                                                            \
  {                                                                                 \
    return name##Expression<EArg>(arg);                                             \
  }

EKAT_UNARY_MATH_EXPRESSION (sqrt,Sqrt)
EKAT_UNARY_MATH_EXPRESSION (exp,Exp)
EKAT_UNARY_MATH_EXPRESSION (log,Log)
EKAT_UNARY_MATH_EXPRESSION (sin,Sin)
EKAT_UNARY_MATH_EXPRESSION (cos,Cos)
#undef EKAT_UNARY_MATH_EXPRESSION

} // namespace ekat

#endif // EKAT_EXPRESSION_MATH_HPP
