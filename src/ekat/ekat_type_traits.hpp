#ifndef EKAT_TYPE_TRAITS_HPP
#define EKAT_TYPE_TRAITS_HPP

#include "ekat/std_meta/ekat_std_type_traits.hpp" // For "remove_all_pointers"

#include <string>
#include <typeinfo>

// A file where we can collect meta-utilities related to type information.

namespace ekat {

// Mini helper struct to detect if a type is a single precision floating point
// Note: EVERYTHING, except T=float would yield value=false.
template<typename T>
struct is_single_precision
{
  enum : bool { value = false };
};

template <> struct is_single_precision<float> { enum : bool { value = true }; };

// Retrieve the underlying value type from a MD array type.
// E.g., ValueType<const double**[3]>::type would be 'const double'
template<typename MDArrayType>
struct ValueType {
private:
  using no_ref = typename std::remove_reference<MDArrayType>::type;
  using no_arr = typename std::remove_all_extents<no_ref>::type;
public:
  using type = typename remove_all_pointers<no_arr>::type;
};

// A structure to find the rank and the dynamic rank of an MD array type.
// E.g., if DataType is double**[3], we'll have rank=3, and rank_dynamic=2
template<typename DataType>
struct GetRanks {
  enum : int { rank = 0 };
  enum : int { rank_dynamic = 0 };
};

template<typename T, int N>
struct GetRanks<T[N]> {
  enum : int { rank = 1 + GetRanks<T>::rank };
  enum : int { rank_dynamic = GetRanks<T>::rank_dynamic };
};

template<typename T>
struct GetRanks<T*> {
  enum : int { rank = 1 + GetRanks<T>::rank };
  enum : int { rank_dynamic = 1+ GetRanks<T>::rank_dynamic };
};

// The following meat-utility establish whether an overload of certain functions
// exists for a particular (set of) type(s).
// Note: the used here is taken from https://stackoverflow.com/a/50631844/1093346

// Equality operator
template <typename LHS, typename RHS>
struct EqualExists {
  template<typename T, typename U>
  static auto test(T &&t, U &&u) -> decltype(t == u, void(), std::true_type{});
  static auto test(...) -> std::false_type;
  using type = decltype(test(std::declval<LHS>(),std::declval<RHS>()));

  static constexpr bool value = type::value;
};

// Streaming operator
template<typename T>
struct StreamExists {
  template<typename U>
  static auto test(U*)
    -> decltype(
        std::declval<std::ostream&>() << std::declval<U>(),
        std::true_type());

  template<typename>
  static std::false_type test(...);

  using type = decltype(test<T>(0));

  static constexpr bool value = type::value;
};

} // namespace ekat

#endif // EKAT_TYPE_TRAITS_HPP
