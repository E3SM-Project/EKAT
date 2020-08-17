#ifndef EKAT_SCALAR_TRAITS_HPP
#define EKAT_SCALAR_TRAITS_HPP

#include "ekat/ekat_assert.hpp"

#include <Kokkos_Core.hpp>

#include <limits>
#include <climits>
#include <typeinfo>

#ifdef KOKKOS_ENABLE_CUDA
#include <math_constants.h>
#endif

namespace ekat {

/*
 * A struct to hold some traits information on certain scalar types
 * 
 * The traits include some utility function (like quiet_NaN() or invalid),
 * as well as some type information, such as whether the template argument
 * is a simd type.
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
                 "Error! Template parameter 'T' in generic ScalarTraits must be a numeric type.\n");

  // Whether this type is a simd type
  static constexpr bool is_simd = false;

  // A human-readable type for T. This generic class defaults to type_info impl.
  static std::string name () {
    return typeid(T).name();
  }

  // ===== Special values for T ===== //

  KOKKOS_INLINE_FUNCTION
  static const value_type quiet_NaN () {
    EKAT_KERNEL_ASSERT_MSG(std::is_floating_point<value_type>::value,
                       "Error! NaN is only available for floating point types.\n");
#ifdef __CUDA_ARCH__
    if (std::is_same<value_type,float>::value) {
      return CUDART_NAN_F;
    } else if (std::is_same<value_type,double>::value) {
      return CUDART_NAN;
    } else {
      EKAT_KERNEL_ERROR_MSG ("Error! No NaN provided for this floating point type.\n");
      // Silence compiler warning
      return 0;
    }
#else
    return std::numeric_limits<value_type>::quiet_NaN();
#endif
  }

  KOKKOS_INLINE_FUNCTION
  static const value_type invalid () {
    // For a floating point, return NaN. For an integer, return largest possible number
    value_type val(0);

    if (std::is_floating_point<value_type>::value) {
      val = ScalarTraits<value_type>::quiet_NaN();
    } else {
      // If cuda supported numeric_limits, we would not need all these ifs
      if (std::is_same<value_type,int>::value) {
        val = static_cast<value_type>(INT_MAX);
      } else if (std::is_same<value_type,unsigned int>::value) {
        val = static_cast<value_type>(UINT_MAX);
      } else if (std::is_same<value_type,long>::value) {
        val = static_cast<value_type>(LONG_MAX);
      } else if (std::is_same<T,unsigned long>::value) {
        val = static_cast<value_type>(ULONG_MAX);
      } else if (std::is_same<value_type,long long>::value) {
        val = static_cast<value_type>(LLONG_MAX);
      } else if (std::is_same<value_type,unsigned long long>::value) {
        val = static_cast<value_type>(ULLONG_MAX);
      }
    }
    return val;
  }
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
