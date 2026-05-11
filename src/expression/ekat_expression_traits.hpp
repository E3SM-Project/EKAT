#ifndef EKAT_EXPRESSION_TRAITS_HPP
#define EKAT_EXPRESSION_TRAITS_HPP

#include <type_traits>

namespace ekat {

// TODO: C++20 comes with std::type_identity<T>
template <typename T>
struct identity { using type = T; };

// Meta-utilities for expressions
template<typename T, typename = void>
struct is_expr : std::false_type {};

template <typename T>
struct is_expr<T, std::void_t<typename T::expression_tag>> : std::true_type {};

template <typename T>
inline constexpr bool is_expr_v = is_expr<T>::value;

template <typename... Ts>
inline constexpr bool is_any_expr_v = (is_expr_v<Ts> || ...);

// Primary template: Handle raw scalars (non-expressions)
template <typename T, bool IsExpr = false>
struct eval_return : identity<T> {};

// Specialization: For when IsExpr is 'true'
template <typename T>
struct eval_return<T, true> {
  // Hard contract: T must have return_type or this will fail loudly
  using type = typename T::return_type;
};

template <typename T>
using eval_return_t = typename eval_return<T,is_expr_v<T>>::type;

// If type is an expression, extract inner type, otherwise return input type
template<typename T, bool IsExpr = false>
struct get_expr_node_trait : identity<T> {};

template<template<typename> class Base, typename D>
struct get_expr_node_trait<Base<D>, true> : identity<D> {};

template<typename T>
using get_expr_node_t = typename get_expr_node_trait<T>::type;

// Sometimes we need to access the concrete derived expression,
// as they are the ONLY types that define return_type (we can't
// access derived's type from the base class, even in CRTP.
template<typename T>
auto get_expr_node (const T& t) {
if constexpr (is_expr_v<T>) {
    return static_cast<const get_expr_node_t<T>&>(t);
  } else {
    return t;
  }
}


} // namespace ekat

#endif // EKAT_EXPRESSION_TRAITS_HPP
