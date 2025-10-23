#ifndef EKAT_SUBVIEW_UTILS_HPP
#define EKAT_SUBVIEW_UTILS_HPP

#include "ekat_kokkos_types.hpp"
#include "ekat_kokkos_meta.hpp"

namespace ekat {

// ================ Subviews of several ranks views ======================= //

// Note: we template on scalar type ST to allow both builtin and Packs,
//       as well as to allow const/non-const versions.

// --- Rank1 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST,Props...>>
subview(const ViewLR<ST*,Props...>& v,
        const int i0) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  return Unmanaged<ViewLR<ST,Props...>>(
      &v.impl_map().reference(i0, 0));
}

// --- Rank2 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*,Props...>>
subview(const ViewLR<ST**,Props...>& v,
        const int i0) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  return Unmanaged<ViewLR<ST*,Props...>>(
      &v.impl_map().reference(i0, 0),v.extent(1));
}

// --- Rank3 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST**,Props...>>
subview(const ViewLR<ST***,Props...>& v,
        const int i0) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  return Unmanaged<ViewLR<ST**,Props...>>(
      &v.impl_map().reference(i0, 0, 0),v.extent(1),v.extent(2));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*,Props...>>
subview(const ViewLR<ST***,Props...>& v,
        const int i0, const int i1) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  return Unmanaged<ViewLR<ST*,Props...>>(
      &v.impl_map().reference(i0, i1, 0),v.extent(2));
}

// --- Rank4 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST***,Props...>>
subview(const ViewLR<ST****,Props...>& v,
        const int i0) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  return Unmanaged<ViewLR<ST***,Props...>>(
      &v.impl_map().reference(i0, 0, 0, 0),v.extent(1),v.extent(2),v.extent(3));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST**,Props...>>
subview(const ViewLR<ST****,Props...>& v,
        const int i0, const int i1) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  return Unmanaged<ViewLR<ST**,Props...>>(
      &v.impl_map().reference(i0, i1, 0, 0),v.extent(2),v.extent(3));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*,Props...>>
subview(const ViewLR<ST****,Props...>& v,
        const int i0, const int i1, const int i2) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  assert(i2>=0 && i2 < v.extent_int(2));
  return Unmanaged<ViewLR<ST*,Props...>>(
      &v.impl_map().reference(i0, i1, i2, 0),v.extent(3));
}

// --- Rank5 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST****,Props...>>
subview(const ViewLR<ST*****,Props...>& v,
        const int i0) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  return Unmanaged<ViewLR<ST****,Props...>>(
      &v.impl_map().reference(i0, 0, 0, 0, 0),v.extent(1),v.extent(2),v.extent(3),v.extent(4));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST***,Props...>>
subview(const ViewLR<ST*****,Props...>& v,
        const int i0, const int i1) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  return Unmanaged<ViewLR<ST***,Props...>>(
      &v.impl_map().reference(i0, i1, 0, 0, 0),v.extent(2),v.extent(3),v.extent(4));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST**,Props...>>
subview(const ViewLR<ST*****,Props...>& v,
        const int i0, const int i1, const int i2) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  assert(i2>=0 && i2 < v.extent_int(2));
  return Unmanaged<ViewLR<ST**,Props...>>(
      &v.impl_map().reference(i0, i1, i2, 0 , 0),v.extent(3),v.extent(4));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*,Props...>>
subview(const ViewLR<ST*****,Props...>& v,
        const int i0, const int i1, const int i2, const int i3) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  assert(i2>=0 && i2 < v.extent_int(2));
  assert(i3>=0 && i3 < v.extent_int(3));
  return Unmanaged<ViewLR<ST*,Props...>>(
      &v.impl_map().reference(i0, i1, i2, i3 , 0),v.extent(4));
}

// --- Rank6 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*****,Props...>>
subview(const ViewLR<ST******,Props...>& v,
        const int i0) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  return Unmanaged<ViewLR<ST*****,Props...>>(
      &v.impl_map().reference(i0, 0, 0, 0, 0, 0),v.extent(1),v.extent(2),v.extent(3),v.extent(4),v.extent(5));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST****,Props...>>
subview(const ViewLR<ST******,Props...>& v,
        const int i0, const int i1) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  return Unmanaged<ViewLR<ST****,Props...>>(
      &v.impl_map().reference(i0, i1, 0, 0, 0, 0),v.extent(2),v.extent(3),v.extent(4),v.extent(5));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST***,Props...>>
subview(const ViewLR<ST******,Props...>& v,
        const int i0, const int i1, const int i2) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  assert(i2>=0 && i2 < v.extent_int(2));
  return Unmanaged<ViewLR<ST***,Props...>>(
      &v.impl_map().reference(i0, i1, i2, 0, 0, 0),v.extent(3),v.extent(4),v.extent(5));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST**,Props...>>
subview(const ViewLR<ST******,Props...>& v,
        const int i0, const int i1, const int i2, const int i3) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  assert(i2>=0 && i2 < v.extent_int(2));
  assert(i3>=0 && i3 < v.extent_int(3));
  return Unmanaged<ViewLR<ST**,Props...>>(
      &v.impl_map().reference(i0, i1, i2, i3, 0, 0),v.extent(4),v.extent(5));
}
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*,Props...>>
subview(const ViewLR<ST******,Props...>& v,
        const int i0, const int i1, const int i2, const int i3, const int i4) {
  assert(v.data() != nullptr);
  assert(i0>=0 && i0 < v.extent_int(0));
  assert(i1>=0 && i1 < v.extent_int(1));
  assert(i2>=0 && i2 < v.extent_int(2));
  assert(i3>=0 && i3 < v.extent_int(3));
  assert(i4>=0 && i4 < v.extent_int(4));
  return Unmanaged<ViewLR<ST*,Props...>>(
      &v.impl_map().reference(i0, i1, i2, i3, i4, 0),v.extent(5));
}

// ================ Subviews along 2nd dimension of a LayoutRight view ======================= //

// Note: If the input view has rank=2, then the output view MUST have LayoutStride (there
//       is no alternative), and, for input rank=3, Kokkos::subview will return a LayoutRight view
//       (using padded layout). For input rank>3, these subviews can, in theory, retain LayoutRight.
//       However, Kokkos::subview only returns LayoutStride and we do not currently have a way to
//       work around this. See https://github.com/kokkos/kokkos/issues/3757.

// --- Rank2 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST*,Props...>>
subview_1(const ViewLR<ST**,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  auto sv = Kokkos::subview(v,Kokkos::ALL,i1);
  return Unmanaged<ViewLS<ST*,Props...>>(sv);
}

// --- Rank3 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST**,Props...>>
subview_1(const ViewLR<ST***,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  auto sv = Kokkos::subview(v,Kokkos::ALL,i1,Kokkos::ALL);
  return Unmanaged<ViewLR<ST**,Props...>>(sv);
}

// --- Rank4 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST***,Props...>>
subview_1(const ViewLR<ST****,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  auto sv = Kokkos::subview(v,Kokkos::ALL,i1,Kokkos::ALL,Kokkos::ALL);
  return Unmanaged<ViewLS<ST***,Props...>>(sv);
}

// --- Rank5 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST****,Props...>>
subview_1(const ViewLR<ST*****,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  auto sv = Kokkos::subview(v,Kokkos::ALL,i1,Kokkos::ALL,Kokkos::ALL,Kokkos::ALL);
  return Unmanaged<ViewLS<ST****,Props...>>(sv);
}

// --- Rank6 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST*****,Props...>>
subview_1(const ViewLR<ST******,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  auto sv = Kokkos::subview(v,Kokkos::ALL,i1,Kokkos::ALL,Kokkos::ALL,Kokkos::ALL,Kokkos::ALL);
  return Unmanaged<ViewLS<ST*****,Props...>>(sv);
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
