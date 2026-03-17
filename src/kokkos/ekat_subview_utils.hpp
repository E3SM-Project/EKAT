#ifndef EKAT_SUBVIEW_UTILS_HPP
#define EKAT_SUBVIEW_UTILS_HPP

#include "ekat_kokkos_types.hpp"
#include "ekat_kokkos_meta.hpp"

#include <cassert>

namespace ekat {

// Helper functions for subviews
namespace Impl {
template <typename ViewT, std::size_t... Is, typename... Indices>
KOKKOS_INLINE_FUNCTION
auto subview_impl(const ViewT& v,
                  std::index_sequence<Is...>,
                  Indices&&... indices) {
  return Kokkos::subview(v, indices...,
                         ((void)Is, Kokkos::ALL)...); // Fold expression to add ALL for remaining dims
}

template <typename ViewT, std::size_t... Is>
KOKKOS_INLINE_FUNCTION
auto subview_1_impl(const ViewT& v,
                    int i1,
                    std::index_sequence<Is...>) {
  return Kokkos::subview(v,
                         Kokkos::ALL, i1,
                         ((void)Is, Kokkos::ALL)...); // Fold expression to add ALL for remaining dims
}
} // namespace Impl

// ================ Subviews of several ranks views ======================= //

// Function for taking Kokkos::subview(v, i0, i1, ... iR, Kokkos::ALL, Kokkos::ALL, ...))
//
// Example (rank-4): subview(v, i0, i1) == v(i0, i1, :, :)
//
// Notes: For layout_right views, subview will retain layout_right

template <typename ViewT, typename... Indices>
KOKKOS_INLINE_FUNCTION
auto subview(const ViewT& v, Indices... indices)
{
  static_assert(sizeof...(Indices) > 0,
                "Error! Must provide at least one index.\n");
  static_assert(sizeof...(Indices) <= ViewT::rank,
                "Error! Number of indices cannot exceed view rank.\n");
  static_assert((std::is_integral_v<std::remove_cv_t<std::remove_reference_t<Indices>>> && ...),
                "Error! All indices must be integral types.\n");
  assert(v.data() != nullptr);

  constexpr size_t num_all_dims = ViewT::rank - sizeof...(Indices);
  auto sview = Impl::subview_impl(v, std::make_index_sequence<num_all_dims>{}, indices...);
  return Unmanaged<decltype(sview)>(sview);
}

// ================ Subviews along 2nd dimension ======================= //

// Function for taking Kokkos::subview(v, :, i1, :, ..., :)
//
// For layout_right views, only rank 3 input views will retain layout_right,
// the rest will get layout_stride. This is due to the fact that Kokkos will
// assign the 2D subview to layout_right_padded, which represents a special case.

template <typename ViewT>
auto subview_1(const ViewT& v,
               const int i1) {
  static_assert(ViewT::rank >= 2, "Error! subview_1 only works for views of rank 2 or higher.\n");
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  auto sview = Impl::subview_1_impl(v, i1, std::make_index_sequence<ViewT::rank - 2>{});
  return Unmanaged<decltype(sview)>(sview);
}

// ================ Multi-sliced Subviews ======================= //
// e.g., instead of a single-entry slice like v(:, 42, :), we slice over a range
// of values, as in v(:, 27:42, :)
// Note that this obtains entries for which in dimesion 2 is in the
// range [27, 42) == {v(i, j, k), where 27 <= j < 42}
// Note also that this slicing means that the subview has the same rank
// as the source view

// --- Rank1 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST*, Props...>>
subview(const ViewLR<ST*, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim = 0) {
  assert(v.data() != nullptr);
  assert(idim == 0);
  assert(kp0.first >= 0 && kp0.first < kp0.second);
  return Unmanaged<ViewLS<ST*,Props...>>(Kokkos::subview(v, kp0));
}

// --- Rank2 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST**, Props...>>
subview(const ViewLR<ST**, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim) {
  assert(v.data() != nullptr);
  assert(idim >= 0 && idim < static_cast<int>(v.rank));
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && kp0.second <= v.extent_int(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST**,Props...>>(Kokkos::subview(v, kp0, Kokkos::ALL));
  } else {
    assert(idim == 1);
    return Unmanaged<ViewLS<ST**,Props...>>(Kokkos::subview(v, Kokkos::ALL, kp0));
  }
}

// --- Rank3 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST***, Props...>>
subview(const ViewLR<ST***, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim) {
  assert(v.data() != nullptr);
  assert(idim >= 0 && idim < static_cast<int>(v.rank));
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && kp0.second <= v.extent_int(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST***,Props...>>(
      Kokkos::subview(v, kp0, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 1) {
    return Unmanaged<ViewLS<ST***,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, kp0, Kokkos::ALL));
  } else {
    assert(idim == 2);
    return Unmanaged<ViewLS<ST***,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, kp0));
  }
}

// --- Rank4 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST****, Props...>>
subview(const ViewLR<ST****, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim) {
  assert(v.data() != nullptr);
  assert(idim >= 0 && idim < static_cast<int>(v.rank));
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && kp0.second <= v.extent_int(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, kp0, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 1) {
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, kp0, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 2) {
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, kp0, Kokkos::ALL));
  } else {
    assert(idim == 3);
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, kp0));
  }
}

// --- Rank5 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST*****, Props...>>
subview(const ViewLR<ST*****, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim) {
  assert(v.data() != nullptr);
  assert(idim >= 0 && idim < static_cast<int>(v.rank));
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && kp0.second <= v.extent_int(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST*****,Props...>>(
      Kokkos::subview(v, kp0, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 1) {
    return Unmanaged<ViewLS<ST*****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, kp0, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 2) {
    return Unmanaged<ViewLS<ST*****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, kp0, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 3) {
    return Unmanaged<ViewLS<ST*****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, kp0, Kokkos::ALL));
  } else {
    assert(idim == 4);
    return Unmanaged<ViewLS<ST*****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, kp0));
  }
}

// --- Rank6 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST******, Props...>>
subview(const ViewLR<ST******, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim) {
  assert(v.data() != nullptr);
  assert(idim >= 0 && idim < static_cast<int>(v.rank));
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && kp0.second <= v.extent_int(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST******,Props...>>(
      Kokkos::subview(v, kp0, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL,
                      Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 1) {
    return Unmanaged<ViewLS<ST******,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, kp0, Kokkos::ALL, Kokkos::ALL,
                      Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 2) {
    return Unmanaged<ViewLS<ST******,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, kp0, Kokkos::ALL,
                      Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 3) {
    return Unmanaged<ViewLS<ST******,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, kp0,
                      Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 4) {
    return Unmanaged<ViewLS<ST******,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL,
                      kp0, Kokkos::ALL));
  } else {
    assert(idim == 5);
    return Unmanaged<ViewLS<ST******,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL,
                      Kokkos::ALL, kp0));
  }
}
} // namespace ekat

#endif // EKAT_SUBVIEW_UTILS_HPP
