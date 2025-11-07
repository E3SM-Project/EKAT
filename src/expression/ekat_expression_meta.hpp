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

} // namespace ekat

#endif // EKAT_EXPRESSION_META_HPP
