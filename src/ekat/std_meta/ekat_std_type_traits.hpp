#ifndef EKAT_STD_TYPE_TRAITS_HPP
#define EKAT_STD_TYPE_TRAITS_HPP

/*
 *  Emulating c++1z features
 *
 *  This file contains some features that belongs to the std namespace,
 *  and that either are likely (if not already scheduled) to be in future standards,
 *  or that conceptually should belong in the std namespace.
 *  Since we have limited access to c++1z standard on target platforms,
 *  we can only rely on c++11 features. Everything else that we may need,
 *  we try to emulate here.
 */

#include <type_traits>

namespace ekat {

// =============== type_traits utils ============== //

// <type_traits> has remove_all_extents but lacks the analogous
// remove_all_pointers
template<typename T>
struct remove_all_pointers {
  using type = T;
};
template<typename T>
struct remove_all_pointers<T*> {
  using type = typename remove_all_pointers<T>::type;
};

// std::remove_const does not remove the leading const cv from
// the type <const T*>. Indeed, remove_const can only remove the
// const cv from the pointer, not the pointee.
template<typename T>
struct remove_all_consts : std::remove_const<T> {};

template<typename T>
struct remove_all_consts<T*> {
  using type = typename remove_all_consts<T>::type*;
};

// The datatype of an N-dim dynamic array, given the value type and N
template<typename T, int N>
struct DataND {
  using type = typename DataND<T,N-1>::type*;
};
template<typename T>
struct DataND<T,1> {
  using type = T*;
};

} // namespace ekat

#endif // EKAT_STD_TYPE_TRAITS_HPP
