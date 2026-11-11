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
