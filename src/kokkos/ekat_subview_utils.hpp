#ifndef EKAT_SUBVIEW_UTILS_HPP
#define EKAT_SUBVIEW_UTILS_HPP

#include "ekat_kokkos_types.hpp"
#include "ekat_kokkos_meta.hpp"

#include <cassert>

namespace ekat {

namespace Impl {

// Return subview of v with subview dims described by args
// Input:
//   - ViewT v: view to subview
//   - std::integer_sequence<int, Is...>: integer sequence [0, 1, ..., ViewT::rank]
//   - Args... args: subview dims for the first sizeof(Args) dims (<= ViewT::rank)
template <typename ViewT, int... Is, class... Args>
KOKKOS_INLINE_FUNCTION
auto subview_impl(const ViewT& v, std::integer_sequence<int, Is...>, const Args... args) {
  // Pack the integral arguments into a tuple so they can be indexed via std::get inside the lambda
  auto args_tuple = std::forward_as_tuple(args...);
  constexpr auto num_args = std::tuple_size_v<decltype(args_tuple)>;

  // Determine the subview arg (args... and then Kokkos::ALL for remaining ranks)
  auto slice = [&](auto dim) {
    if constexpr (dim < (int)num_args) {
      auto indx = std::get<dim>(args_tuple);
      assert((int)indx < v.extent_int((int)dim));

      // For the first r dimensions, extract the runtime integer parameter
      return std::get<dim>(args_tuple);
    } else {
      // For the remaining dimensions, pass Kokkos::ALL
      return Kokkos::ALL;
    }
  };
  return Kokkos::subview(v, slice(std::integral_constant<int, Is>())...);
}

// Return subview of v at index i1 of dim1
// Input:
//   - ViewT v: view to subview
//   - int i1: subview index for dim1
//   - std::integer_sequence<int, Is...>: integer sequence [0, 1, ..., ViewT::rank]
template <typename ViewT, int... Is>
KOKKOS_INLINE_FUNCTION
auto subview_1_impl(const ViewT& v, const int i1, std::integer_sequence<int, Is...>) {
  auto slice = [&](auto dim) {
    if constexpr (dim == 1) {
      return i1;
    } else {
      return Kokkos::ALL;
    }
  };

  return Kokkos::subview(v, slice(std::integral_constant<int, Is>{})...);
}

} // namespace Impl

// ================ Subviews of first r ranks ======================= //
// Return subview of v with subview dims described by args
// Input:
//   - ViewT v: view to subview
//   - Args... args: subview dims for the first sizeof(Args) dims (<= ViewT::rank)
template <typename ViewT, typename... Args>
requires((ViewT::rank >= sizeof...(Args)) &&
         (std::is_integral_v<Args> && ...) &&
         (std::convertible_to<Args, int> && ...))
KOKKOS_INLINE_FUNCTION
auto subview(const ViewT& v, const Args... args) {
  assert(v.data() != nullptr);

  auto int_seq = std::make_integer_sequence<int, ViewT::rank>();
  auto sv = Impl::subview_impl(v, int_seq, args...);

  using Subview = decltype(sv);
  return Unmanaged<Subview>(sv);
}

// ================ Subviews along 2nd dimension ======================= //

template <typename ViewT>
requires (ViewT::rank > 1)
KOKKOS_INLINE_FUNCTION
auto subview_1(const ViewT& v, const int i1) {
  assert(v.data() != nullptr);
  assert(i1 >= 0 && i1 < v.extent_int(1));

  // Pass in a compile-time sequence matching the rank of the View
  auto sv = Impl::subview_1_impl(v, i1, std::make_integer_sequence<int, ViewT::rank>{});
  return Unmanaged<decltype(sv)>(sv);
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
