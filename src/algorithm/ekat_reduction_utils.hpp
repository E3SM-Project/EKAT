#ifndef EKAT_REDUCTION_UTILS_HPP
#define EKAT_REDUCTION_UTILS_HPP

#include "ekat_kokkos_meta.hpp"
#include "ekat_kokkos_types.hpp"
#include "ekat_type_traits.hpp"
#include "ekat_std_type_traits.hpp"
#include "ekat_scalar_traits.hpp"
#include "ekat_type_traits.hpp"
#include "ekat_pack.hpp"

namespace ekat {

namespace impl {

/*
 * Computes a general parallel reduction. If Serialize=true, this reduction is computed
 * one element in order, useful for BFB testing with serial routines.
 * If Serialize=false, this function simply calls Kokkos::parallel_reduce().
 */
template <bool Serialize, typename ValueType, typename TeamMember, typename Lambda>
static KOKKOS_INLINE_FUNCTION
ValueType parallel_reduce (const TeamMember& team,
                           const int& begin,
                           const int& end,
                           const Lambda& lambda)
{
  // NOTE: initialization only necessary for the Serialize=true case.
  ValueType result = Kokkos::reduction_identity<ValueType>::sum();
  if (Serialize) {
    // Perform the reduction serially. All threads perform the same calculation,
    // (ok, since the result var is a thread-private automatic variable).
    for (int k=begin; k<end; ++k) {
      lambda(k, result);
    }
  } else {
    // Use the whole team of threads
    Kokkos::parallel_reduce(Kokkos::TeamThreadRange(team, begin, end), lambda, result);
  }
  return result;
}

/*
 * Computes a reduction over the range [begin,end) of items provided by the InputProvider
 * object, which must support calls to operator()(int).
 * We provide two overloads, one for InputProvider returning a simd-type, and one for
 * InputProvider returning a non-simd type. In both cases, the result of the reduction
 * is a non-simd type, and the [begin,end) range refers to the 'scalar' range.
 * NOTE: for simd-type usage, the simd-type must a) have a specialization of ekat::ScalarTraits
 *       that sets is_simd=true, b) support operator[](int), and c) have a compile-time size
 *       equal to the length of the simd array times the size of the individual element type.
 *       Furthermore, a specialization of Kokkos::reduction_identity<T> must be available for
 *       the simd type T.
 * NOTE: we do not provide an overload with Serialized defaulted, since this fcn is an impl
 *       detail, and, normally, should not be used by customer apps
 */
template<typename InputProvider>
using ResultType = typename std::remove_cv<
                      typename std::remove_reference<
                          decltype( std::declval<InputProvider>()(0) )
                      >::type
                   >::type;

template<typename InputProvider>
using ResultTraits = ekat::ScalarTraits<ekat::impl::ResultType<InputProvider>>;

template <bool Serialize, typename TeamMember, typename InputProvider>
static KOKKOS_INLINE_FUNCTION
auto view_reduction (const TeamMember& team,
                     const int begin, // scalar index
                     const int end, // scalar index
                     const InputProvider& input)
 -> typename std::enable_if<not ekat::impl::ResultTraits<InputProvider>::is_simd,
                            ekat::impl::ResultType<InputProvider>>::type
{
  using ValueType = typename ekat::impl::ResultType<InputProvider>;
  auto lambda = [&](const int k, ValueType& local_sum) {
    local_sum += input(k);
  };
  return impl::parallel_reduce<Serialize,ValueType>(team, begin, end, lambda);
}

template <bool Serialize, typename TeamMember, typename InputProvider>
static KOKKOS_INLINE_FUNCTION
auto view_reduction (const TeamMember& team,
                     const int begin, // scalar index
                     const int end, // scalar index
                     const InputProvider& input)
 -> typename std::enable_if<ekat::impl::ResultTraits<InputProvider>::is_simd,
                            typename ekat::impl::ResultTraits<InputProvider>::scalar_type>::type
{
  using PackType = ekat::impl::ResultType<InputProvider>;
  using ValueType = typename ekat::impl::ResultTraits<InputProvider>::scalar_type;
  constexpr int N = sizeof(PackType) / sizeof(ValueType);

  // For the serialized case, "unpack" the input provider result, and call the scalar version
  ValueType result = Kokkos::reduction_identity<ValueType>::sum();
  if (Serialize) {
    auto scalar_input = [&](const int k) {
      return input(k/N)[k%N];
    };
    result = view_reduction<true>(team,begin,end,scalar_input);
  } else {
    // For packed case, check if we need to treat first/last packs separately

    const bool has_garbage_begin = begin % N != 0;
    const bool has_garbage_end   =   end % N != 0;
    const int  pack_loop_begin   = (has_garbage_begin ? begin/N + 1 : begin/N);
    const int  pack_loop_end     = end/N;

    // If first pack has garbage, we will not include it in the below reduction,
    // so manually add the first pack (only the non-garbage part)
    if (has_garbage_begin) {
      const auto temp_input = input(pack_loop_begin-1);
      const int first_indx = begin % N;
      for (int j=first_indx; j<N; ++j) {
        result += temp_input[j];
      }
    }

    // Complete packs to be reduced. If Serialize, reduce each pack first, then
    // sum into result. Else, sum up packs, then reduce the packed sum into result.
    if (pack_loop_begin != pack_loop_end) {
      auto temp = impl::parallel_reduce<false,PackType>(
          team, pack_loop_begin, pack_loop_end,
          [&](const int k, PackType& local_packed_sum) {
            // Sum packs
            local_packed_sum += input(k);
      });
      result += ekat::reduce_sum<false>(temp);
    }

    // If last pack has garbage, we did not include it in the main reduction,
    // so manually add the last pack (only the non-garbage part)
    if (has_garbage_end) {
      const PackType temp_input = input(pack_loop_end);
      const int last_indx = end % N;
      for (int j=0; j<last_indx; ++j) {
        result += temp_input[j];
      }
    }
  }
  return result;
}

} //namespace impl

/*
 * ReductionUtils is a wrapper to the implementations above.
 * Uses ekatBFB as default for whether to Serialize reductions or not.
 * NOTE: in case the return type of the input provide is a simd type,
 *       begin/end indices should reflect the *scalar* bounds of the
 *       reduction, so that the impl will NOT include possible padding
 *       values in the tail.
 */
template <typename ExecSpace, bool Serialize = ekatBFB>
struct ReductionUtils {

  template <typename ValueType, typename TeamMember, typename InputProvider>
  static KOKKOS_INLINE_FUNCTION
  ValueType parallel_reduce (const TeamMember& team,
                             const int& begin, // scalar index
                             const int& end,   // scalar index
                             const InputProvider& input)
  {
    return impl::parallel_reduce<Serialize, ValueType>(team, begin, end, input);
  }

  template <typename TeamMember, typename InputProvider>
  static KOKKOS_INLINE_FUNCTION
  auto view_reduction (const TeamMember& team,
                       const int& begin, // scalar index
                       const int& end,   // scalar index
                       const InputProvider& input)
   -> typename ekat::impl::ResultTraits<InputProvider>::scalar_type
  {
    return impl::view_reduction<Serialize>(team,begin,end,input);
  }
};

} // namespace ekat

#endif // EKAT_REDUCTION_UTILS_HPP
