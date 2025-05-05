#ifndef EKAT_SCALAR_TRAITS_HPP
#define EKAT_SCALAR_TRAITS_HPP

#include <type_traits>

namespace ekat {

/*
 * A struct to hold traits information of a scalar type
 * 
 * The traits include type information, such as whether the template argument
 * is a simd type, or what is the value type.
 */

template<typename T>
struct ScalarTraits {
  // It's important to distinguish between value_type and scalar_type.
  //  - value_type is the type of T (stripped of cv qualifiers and reference, see partial specializations below).
  //  - scalar_type is the building block that makes up T.
  // For built-in scalars (float, int, double,...) they coincide. However,
  // for "vector-like" scalars (e.g., a simd type), they will differ.
  // For instance:
  //   using Vector3D = const std::array<double,3>;
  //   using VT = ScalarTraits<Vector3D>::value_type;
  //   using ST = ScalarTraits<Vector3D>::scalar_type;
  // In the above, VT=std::array<double,3>, while ST=double.
  // In other words, scalar_type is whatever makes up T.

  using value_type  = T;
  using scalar_type = value_type;

  // This non-specialized Traits struct is only meant for arithmetic types.
  // Specialization of the class for, say, simd types, might want to perform the
  // check on scalar_type instead (or maybe not even on that, if they allow nested simd types).
  static_assert (std::is_arithmetic<value_type>::value,
                 "Error! Template parameter 'T' in generic ScalarTraits must be an arithmetic type.\n");

  // Whether this type is a simd-like type
  static constexpr bool is_simd = false;
};

// Partial specialization, so that if T has cv qualifiers or reference,
// they get stripped. This way, specializations on different T's, need
// not to have specializations for const/volatile and &.
template<typename T>
struct ScalarTraits<const T> : ScalarTraits<T> {};

template<typename T>
struct ScalarTraits<volatile T> : ScalarTraits<T> {};

template<typename T>
struct ScalarTraits<T&> : ScalarTraits<T> {};

template<typename T>
struct ScalarTraits<T&&> : ScalarTraits<T> {};

} // namespace ekat

#endif // EKAT_SCALAR_TRAITS_HPP
