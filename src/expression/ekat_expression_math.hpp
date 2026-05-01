#ifndef EKAT_EXPRESSION_MATH_HPP
#define EKAT_EXPRESSION_MATH_HPP

#include "ekat_expression_meta.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

// ----------------- Binary math fcns ------------------- //

#define EKAT_BINARY_MATH_EXPRESSION(impl,name) \
  template<typename EArg1, typename EArg2>                                              \
  class name##Expression {                                                              \
  public:                                                                               \
    static constexpr bool expr_l = is_expr_v<EArg1>;                                    \
    static constexpr bool expr_r = is_expr_v<EArg2>;                                    \
                                                                                        \
    /* Don't create an expression from builtin types, just call the math fcn! */        \
    static_assert (expr_l or expr_r,                                                    \
      "At least one between EArg1 and EArg2 must be an Expression type.\n");            \
                                                                                        \
    using eval_arg1_t = eval_return_t<EArg1>;                                           \
    using eval_arg2_t = eval_return_t<EArg2>;                                           \
    using eval_t = std::common_type_t<eval_arg1_t,eval_arg2_t>;                         \
                                                                                        \
    KOKKOS_INLINE_FUNCTION                                                              \
    name##Expression (const EArg1& arg1, const EArg2& arg2)                             \
      : m_arg1(arg1)                                                                    \
      , m_arg2(arg2)                                                                    \
    {}                                                                                  \
                                                                                        \
    static constexpr int rank () {                                                      \
      if constexpr (expr_l and expr_r) {                                                \
        constexpr int lr = EArg1::rank();                                               \
        constexpr int rr = EArg2::rank();                                               \
        if constexpr (lr > 0 and rr > 0) {                                              \
          static_assert(lr == rr,                                                       \
            "[BinaryMathExpression] Error! EArg1 and EArg2 have different rank.\n");    \
        }                                                                               \
        return lr > rr ? lr : rr;                                                       \
      } else if constexpr (expr_l) {                                                    \
        return EArg1::rank();                                                           \
      } else {                                                                          \
        return EArg2::rank();                                                           \
      }                                                                                 \
    }                                                                                   \
    KOKKOS_INLINE_FUNCTION                                                              \
    int extent (int i) const {                                                          \
      if constexpr (expr_l and expr_r) {                                                \
        if constexpr (EArg1::rank() > 0)                                               \
          return m_arg1.extent(i);                                                      \
        else if constexpr (EArg2::rank() > 0)                                          \
          return m_arg2.extent(i);                                                      \
        else                                                                            \
          return 0;                                                                     \
      } else if constexpr (expr_l) {                                                    \
        return m_arg1.extent(i);                                                        \
      } else {                                                                          \
        return m_arg2.extent(i);                                                        \
      }                                                                                 \
    }                                                                                   \
                                                                                        \
    static constexpr ExprKind kind () {                                                 \
      if constexpr (expr_l and expr_r)                                                  \
        return expr_kind_max(EArg1::kind(), EArg2::kind());                             \
      else if constexpr (expr_l)                                                        \
        return EArg1::kind();                                                           \
      else if constexpr (expr_r)                                                        \
        return EArg2::kind();                                                           \
      else                                                                              \
        return ExprKind::Elemental;                                                     \
    }                                                                                   \
    template<typename MemberType>                                                       \
    KOKKOS_INLINE_FUNCTION                                                              \
    void setup (const MemberType& team) {                                               \
      if constexpr (expr_l) m_arg1.setup(team);                                        \
      if constexpr (expr_r) m_arg2.setup(team);                                        \
    }                                                                                   \
                                                                                        \
    template<typename... Args>                                                          \
    KOKKOS_INLINE_FUNCTION                                                              \
    eval_t eval(Args... args) const {                                                   \
      if constexpr (not expr_l)                                                         \
        return eval_impl(m_arg1,m_arg2.eval(args...));                                  \
      else if constexpr (not expr_r)                                                    \
        return eval_impl(m_arg1.eval(args...),m_arg2);                                  \
      else {                                                                            \
        constexpr int lr = EArg1::rank();                                               \
        constexpr int rr = EArg2::rank();                                               \
        if constexpr (lr == 0 and rr > 0)                                               \
          return eval_impl(m_arg1.eval(),m_arg2.eval(args...));                         \
        else if constexpr (rr == 0 and lr > 0)                                         \
          return eval_impl(m_arg1.eval(args...),m_arg2.eval());                         \
        else                                                                            \
          return eval_impl(m_arg1.eval(args...),m_arg2.eval(args...));                  \
      }                                                                                 \
    }                                                                                   \
  protected:                                                                            \
    KOKKOS_INLINE_FUNCTION                                                              \
    eval_t eval_impl(const eval_arg1_t& arg1, const eval_arg2_t& arg2) const {          \
      return Kokkos::impl(static_cast<const eval_t&>(arg1),                             \
                          static_cast<const eval_t&>(arg2));                            \
    }                                                                                   \
                                                                                        \
    EArg1   m_arg1;                                                                     \
    EArg2   m_arg2;                                                                     \
  };                                                                                    \
                                                                                        \
  /* Free function to create a ##nameExpression */                                      \
  template<typename EArg1, typename EArg2>                                              \
  std::enable_if_t<is_expr_v<EArg1> or is_expr_v<EArg2>,name##Expression<EArg1,EArg2>>  \
  impl (const EArg1& arg1, const EArg2& arg2)                                           \
  {                                                                                     \
    return name##Expression<EArg1,EArg2>(arg1,arg2);                                    \
  }                                                                                     \
                                                                                        \
  /* Specialize meta util */                                                            \
  template<typename EArg1, typename EArg2>                                              \
  struct is_expr<name##Expression<EArg1,EArg2>> : std::true_type {};                    \
  template<typename EArg1, typename EArg2>                                              \
  struct eval_return<name##Expression<EArg1,EArg2>> {                                   \
    using type = typename name##Expression<EArg1,EArg2>::eval_t;                        \
  };

EKAT_BINARY_MATH_EXPRESSION(pow,Pow);
EKAT_BINARY_MATH_EXPRESSION(max,Max);
EKAT_BINARY_MATH_EXPRESSION(min,Min);

#undef EKAT_BINARY_MATH_EXPRESSION

// ----------------- Unary math fcns ------------------- //

#define EKAT_UNARY_MATH_EXPRESSION(impl,name) \
  template<typename EArg>                                               \
  class name##Expression {                                              \
  public:                                                               \
    using arg_eval_t = eval_return_t<EArg>;                             \
    using eval_t = decltype(Kokkos::impl(std::declval<arg_eval_t>()));  \
                                                                        \
    KOKKOS_INLINE_FUNCTION                                              \
    name##Expression (const EArg& arg)                                  \
      : m_arg(arg)                                                      \
    {}                                                                  \
                                                                        \
    static constexpr int rank() { return EArg::rank(); }                \
    KOKKOS_INLINE_FUNCTION int extent (int i) const { return m_arg.extent(i); } \
                                                                        \
    static constexpr ExprKind kind () { return EArg::kind(); }          \
    template<typename MemberType>                                       \
    KOKKOS_INLINE_FUNCTION                                              \
    void setup (const MemberType& team) { m_arg.setup(team); }          \
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

EKAT_UNARY_MATH_EXPRESSION (sqrt,Sqrt)
EKAT_UNARY_MATH_EXPRESSION (exp,Exp)
EKAT_UNARY_MATH_EXPRESSION (log,Log)
EKAT_UNARY_MATH_EXPRESSION (sin,Sin)
EKAT_UNARY_MATH_EXPRESSION (cos,Cos)
#undef EKAT_UNARY_MATH_EXPRESSION

} // namespace ekat

#endif // EKAT_EXPRESSION_MATH_HPP
