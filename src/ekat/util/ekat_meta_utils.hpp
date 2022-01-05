#ifndef EKAT_META_UTILS_HPP
#define EKAT_META_UTILS_HPP

#include <tuple>

namespace ekat
{

// A parameter pack. To be used for template specializations
template<typename... Ts>
struct TypeList;

template<typename T>
struct TypeList<T> {
  using head = T;
  static constexpr int size = 1;
};

template<typename T, typename... Ts>
struct TypeList<T,Ts...> {
private:
  using tail = TypeList<Ts...>;
public:
  using head = T;
  static constexpr int size = 1 + tail::size;
};

// Cat two packs
template<typename P1, typename P2>
struct CatLists;

template<typename... T1s, typename... T2s>
struct CatLists<TypeList<T1s...>,TypeList<T2s...>> {
  using type = TypeList<T1s...,T2s...>;
};

// Find position of first occurrence of type T in a TypeList, store in 'pos'.
// If not found, set pos = -1.
template<typename T, typename TypeList>
struct FirstOf;

template<typename T, typename H>
struct FirstOf<T,TypeList<H>> {
  static constexpr int pos = std::is_same<H,T>::value ? 0 : -1;
};

template<typename T, typename H, typename... Ts>
struct FirstOf<T,TypeList<H,Ts...>> {
private:
  static constexpr int tail_pos = FirstOf<T,TypeList<Ts...>>::pos;
public:
  static constexpr int pos = std::is_same<H,T>::value ? 0 : (tail_pos>=0 ? 1+tail_pos : -1);
};

// Check if a TypeList contains unique types
template<typename P>
struct UniqueTypeList;

template<typename T>
struct UniqueTypeList<TypeList<T>> : std::true_type {};

template<typename T, typename... Ts>
struct UniqueTypeList<TypeList<T,Ts...>> {
  static constexpr bool value = FirstOf<T,TypeList<Ts...>>::pos==-1 &&
                                UniqueTypeList<TypeList<Ts...>>::value;
};

// Access N-th entry of a pack
template<typename P, int N>
struct AccessList;

template<typename T, typename... Ts, int N>
struct AccessList <TypeList<T,Ts...>,N> {
private:
  using pack = TypeList<T,Ts...>;
  using tail = TypeList<Ts...>;
  static_assert (N>=0 && N<pack::size, "Error! Index out of bounds.\n");

public:
  using type = typename AccessList<tail,N-1>::type;
};

template<typename T, typename... Ts>
struct AccessList <TypeList<T,Ts...>,0> {
  using type = T;
};

// A map of types
template<typename KP, typename VP>
struct TypeMap;

template<typename... Ks, typename... Vs>
struct TypeMap<TypeList<Ks...>,TypeList<Vs...>> : public std::tuple<Vs...> {
  using keys = TypeList<Ks...>;
  using vals = TypeList<Vs...>;
  using self = TypeMap<keys,vals>;
  static_assert (keys::size==vals::size, "Error! Keys and values have different sizes.\n");
  static_assert (UniqueTypeList<keys>::value, "Error! Keys are not unique.\n");

  template<typename K>
  struct AtHelper {
    static constexpr int pos = FirstOf<K,keys>::pos;
    static_assert(pos>=0, "Error! Input type not found in this TypeMap.\n");
    // For pos<0, expose whatever you want. This helper won't compile anyways
    using type = typename std::conditional<pos<0,void,typename AccessList<vals,pos>::type>::type;
  };

  template<typename K>
  using at_t = typename AtHelper<K>::type;

  template<typename K>
  at_t<K>& at () { return std::get<AtHelper<K>::pos>(*this); }

  static constexpr int size = keys::size;
};

// Given a class template C, and a pack of template args T1,...,Tn,
// create a pack containing the C<T1>,...,C<Tn>
template<template<typename> class C, typename P>
struct ApplyTemplate;

template<template<typename> class C, typename T>
struct ApplyTemplate<C,TypeList<T>> {
  using type = TypeList<C<T>>;
};

template<template<typename> class C, typename T, typename... Ts>
struct ApplyTemplate<C,TypeList<T,Ts...>> {
private:
  using head = TypeList<C<T>>;
  using tail = typename ApplyTemplate<C,TypeList<Ts...>>::type;
public:
  using type = typename CatLists<head,tail>::type;
};

// Iterate over a TypeList
// Requires a generic lambda that only accept an instance of the
// pack types. Can only use input to deduce its type.
//  auto l = [&](auto t) {
//    using T = decltype(t);
//    call_some_func<T>(...);
//  };
template<typename P>
struct TypeListFor;

template<typename T>
struct TypeListFor<TypeList<T>> {
  template<typename Lambda>
  constexpr TypeListFor (Lambda&& l) {
    l(T());
  }
};

template<typename T, typename...Ts>
struct TypeListFor<TypeList<T,Ts...>> {
  template<typename Lambda>
  constexpr TypeListFor (Lambda&& l) {
    l(T());
    TypeListFor<TypeList<Ts...>> pf(l);
  }
};

} // namespace ekat

#endif // EKAT_META_UTILS_HPP
