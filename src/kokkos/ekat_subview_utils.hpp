#ifndef EKAT_SUBVIEW_UTILS_HPP
#define EKAT_SUBVIEW_UTILS_HPP

#include "ekat_kokkos_types.hpp"
#include "ekat_kokkos_meta.hpp"

#include <cassert>

namespace ekat {

namespace impl {
// Helper functions for subview operations
template <typename ViewT, typename... Indices, int... Remaining>
KOKKOS_INLINE_FUNCTION
auto subview_impl(const ViewT& v, std::integer_sequence<int, Remaining...>, Indices... indices) {
  auto result = Kokkos::subview(v, indices...,
                               (static_cast<void>(Remaining), Kokkos::ALL)...);
  return Unmanaged<decltype(result)>(result);
}

template <typename ViewT, int... Remaining>
KOKKOS_INLINE_FUNCTION
auto subview_1_impl(const ViewT& v, const int i1, std::integer_sequence<int, Remaining...>) {
  auto result = Kokkos::subview(v, Kokkos::ALL, i1,
                               (static_cast<void>(Remaining), Kokkos::ALL)...);
  return Unmanaged<decltype(result)>(result);
}

template <typename ViewT, int... Dimensions>
KOKKOS_INLINE_FUNCTION
auto subview_multi_slice_impl(const ViewT& v, const Kokkos::pair<int,int>& range,
                              const int idim, std::integer_sequence<int, Dimensions...>) {
  auto result = Kokkos::subview(v,
      (Dimensions == idim
       ? range : Kokkos::make_pair<int, int>(0, v.extent_int(Dimensions)))...);
  return Unmanaged<decltype(result)>(result);
}
} // namespace impl

// ================ Subviews of several ranks views ======================= //

template <typename ViewT, typename... Indices>
requires (Kokkos::is_view<ViewT>::value && sizeof...(Indices) > 0 &&
          sizeof...(Indices) <= ViewT::rank &&
          (std::is_convertible_v<Indices,int> && ...))
KOKKOS_INLINE_FUNCTION
auto subview(const ViewT& v, Indices... indices) {
  return impl::subview_impl(v, std::make_integer_sequence<int, ViewT::rank - sizeof...(Indices)>{}, indices...);
}

// ================ Subviews along 2nd dimension ======================= //

template <typename ViewT>
requires (Kokkos::is_view<ViewT>::value && ViewT::rank >= 2)
KOKKOS_INLINE_FUNCTION
auto subview_1(const ViewT& v,
               const int i1) {
  return impl::subview_1_impl(v, i1, std::make_integer_sequence<int, ViewT::rank - 2>{});
}

// ================ Multi-sliced Subviews ======================= //
// e.g., instead of a single-entry slice like v(:, 42, :), we slice over a range
// of values, as in v(:, 27:42, :)
// Note that this obtains entries for which in dimesion 2 is in the
// range [27, 42) == {v(i, j, k), where 27 <= j < 42}
// Note also that this slicing means that the subview has the same rank
// as the source view

template <typename ViewT>
requires (Kokkos::is_view<ViewT>::value && ViewT::rank >= 1)
KOKKOS_INLINE_FUNCTION
auto subview(const ViewT& v, const Kokkos::pair<int,int>& range,
             const int idim = 0) {
  assert(idim >= 0 && idim < static_cast<int>(v.rank));
  return impl::subview_multi_slice_impl(v, range, idim, std::make_integer_sequence<int, ViewT::rank>{});
}
} // namespace ekat

#endif // EKAT_SUBVIEW_UTILS_HPP
