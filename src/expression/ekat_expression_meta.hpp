#ifndef EKAT_EXPRESSION_META_HPP
#define EKAT_EXPRESSION_META_HPP

namespace ekat {

// Detect if a type is an Expression
template<typename T>
struct is_expr : std::false_type {};
template<typename T>
constexpr bool is_expr_v = is_expr<T>::value;

template<typename T>
struct eval_return { using type = T; };
template<typename T>
using eval_return_t = typename eval_return<T>::type;

// Classification of expressions by the parallelism they require when evaluated
// inside a team kernel.  Values are ordered so that
//   Elemental < Reduction < TeamParallel
// and composites propagate the maximum of their sub-expressions.
enum class ExprKind {
  Elemental,    // pointwise — no inner parallel work
  Reduction,    // requires a team-level parallel_reduce before eval
  TeamParallel, // requires arbitrary team-level work before eval
};

// Compile-time max of two ExprKind values.
constexpr ExprKind expr_kind_max(ExprKind a, ExprKind b)
{
  return static_cast<ExprKind>(
    static_cast<int>(a) > static_cast<int>(b)
    ? static_cast<int>(a) : static_cast<int>(b));
}

} // namespace ekat

#endif // EKAT_EXPRESSION_META_HPP
