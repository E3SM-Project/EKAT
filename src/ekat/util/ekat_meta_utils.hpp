#ifndef EKAT_META_UTILS_HPP
#define EKAT_META_UTILS_HPP

#include <tuple>
#include <type_traits>

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
    TypeListFor<TypeList<Ts...>> tlf(l);
  }
};

// A map of types
template<typename KP, typename VP>
struct TypeMap;

template<typename... Ks, typename... Vs>
struct TypeMap<TypeList<Ks...>,TypeList<Vs...>> : public std::tuple<Vs...> {
  struct NotFound {};
private:

  template<typename T>
  using base_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

  template<typename V1, typename V2>
  static typename std::enable_if<std::is_same<base_t<V1>,base_t<V2>>::value,bool>::type
  check_eq (const V1& v1, const V2& v2) {
    return v1==v2;
  }
  template<typename V1, typename V2>
  static typename std::enable_if<!std::is_same<base_t<V1>,base_t<V2>>::value,bool>::type
  check_eq (const V1& /* v1 */, const V2& /* v2 */) {
    return false;
  }
public:

  using keys = TypeList<Ks...>;
  using vals = TypeList<Vs...>;
  using self = TypeMap<keys,vals>;
  static_assert (keys::size==vals::size, "Error! Keys and values have different sizes.\n");
  static_assert (UniqueTypeList<keys>::value, "Error! Keys are not unique.\n");

  template<typename K>
  struct AtHelper {
    static constexpr int pos = FirstOf<K,keys>::pos;
  private:
    // The second arg to std::conditional must be instantiated. If pos<0,
    // AccessList does not compile. Hence, if pos<0, use 0, which will for
    // sure work (TypeList has size>=1). The corresponding type will not
    // be exposed anyways. It's just to get a valid instantiation of
    // std::conditional.
    struct GetSafe {
      static constexpr int safe_pos = pos>=0 ? pos : 0;
      using type = typename AccessList<vals,safe_pos>::type;
    };
  public:
    using type = typename std::conditional<(pos<0),NotFound,typename GetSafe::type>::type;
  };

  template<typename K>
  static constexpr bool has_t () {
    return not std::is_same<at_t<K>,NotFound>::value;
  }

  template<typename V>
  bool has_v (const V& v) const {
    return has_v_impl<(FirstOf<V,vals>::pos>=0)>(v,*this);
  }


  template<typename K>
  using at_t = typename AtHelper<K>::type;

  template<typename K>
  at_t<K>& at () {
    static_assert (has_t<K>(), "Error! Type not found in this TypeMap's keys.\n");
    return std::get<AtHelper<K>::pos>(*this);
  }

  template<typename K>
  const at_t<K>& at () const {
    static_assert (has_t<K>(), "Error! Type not found in this TypeMap's keys.\n");
    return std::get<AtHelper<K>::pos>(*this);
  }

  static constexpr int size = keys::size;
private:

  template<bool has_V_type, typename V>
  typename std::enable_if<has_V_type,bool>::type
  has_v_impl (const V& v, const self& map) const {
    bool found = false;
    TypeListFor<keys>([&](auto t){
      using key_t = decltype(t);
      const auto& value = map.at<key_t>();
      if (check_eq(v,value)) {
        found = true;
        return;
      }
    });
    return found;
  }

  template<bool has_V_type, typename V>
  typename std::enable_if<!has_V_type,bool>::type
  has_v_impl (const V& /*v*/, const self& /*map*/) const {
    return false;
  }
};

} // namespace ekat

#endif // EKAT_META_UTILS_HPP
