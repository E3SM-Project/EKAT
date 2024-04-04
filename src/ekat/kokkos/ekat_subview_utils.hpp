#ifndef EKAT_SUBVIEW_UTILS_HPP
#define EKAT_SUBVIEW_UTILS_HPP

#include "ekat/kokkos/ekat_kokkos_types.hpp"
#include "ekat/kokkos/ekat_kokkos_meta.hpp"

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

// ================ Subviews along 2nd dimension ======================= //

// Note: if input rank>2, these subviews can retain LayoutRight.
//       However, Kokkos::subview only works if the output rank is <=2,
//       so for higher ranks, we manually build the output view
//       instead of relying on Kokkos::subview.
//       See https://github.com/kokkos/kokkos/issues/3757
//       If the input view has rank=2, then the output view MUST have
//       LayoutStride (there is no alternative).

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

  using vt = Unmanaged<ViewLR<ST**,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  auto test =  Unmanaged<ViewLR<ST**,Props...>>(v.impl_track(),vm);
  return Unmanaged<ViewLR<ST**,Props...>>(
      v.impl_track(),vm);
}

// --- Rank4 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST***,Props...>>
subview_1(const ViewLR<ST****,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  using vt = Unmanaged<ViewLR<ST***,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2),v.extent(3));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  // auto test =  Unmanaged<ViewLR<ST***,Props...>>(v.impl_track(),vm);
  return Unmanaged<ViewLR<ST***,Props...>>(
      v.impl_track(),vm);
}

// --- Rank5 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST****,Props...>>
subview_1(const ViewLR<ST*****,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  using vt = Unmanaged<ViewLR<ST****,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0,0,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2),v.extent(3),v.extent(4));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  return Unmanaged<ViewLR<ST****,Props...>>(
      v.impl_track(),vm);
}

// --- Rank6 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*****,Props...>>
subview_1(const ViewLR<ST******,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

  using vt = Unmanaged<ViewLR<ST*****,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0,0,0,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2),v.extent(3),v.extent(4),v.extent(5));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  return Unmanaged<ViewLR<ST*****,Props...>>(
      v.impl_track(),vm);
}

// ================ Multi-sliced Subviews ======================= //
// e.g., instead of a single-entry slice like v(:, 42, :), we slice over a range
// of values, as in v(:, 27:42, :)
// this means that the subview has the same rank as the "parent" view

// --- Rank1 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST*, Props...>>
subview(const ViewLR<ST*, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim) {
  assert(v.data() != nullptr);
  assert(idim >= 0 && idim == v.rank);
  // NOTE: the final comparison is originally int <= long unsigned int
  // the cast silences a warning, but may be unnecessary
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && (unsigned int)kp0.second <= v.extent(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST*,Props...>>(Kokkos::subview(v, kp0, Kokkos::ALL));
  } else {
    // FIXME: better way to do this? necessary?
    assert(false);
  }
}

// --- Rank2 multi-slice --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLS<ST**, Props...>>
subview(const ViewLR<ST**, Props...>& v,
        const Kokkos::pair<int, int> &kp0,
        const int idim) {
  assert(v.data() != nullptr);
  assert(idim >= 0 && idim <= v.rank);
  // NOTE: the final comparison is originally int <= long unsigned int
  // the cast silences a warning, but may be unnecessary
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && (unsigned int)kp0.second <= v.extent(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST**,Props...>>(Kokkos::subview(v, kp0, Kokkos::ALL));
  } else if (idim == 1) {
    return Unmanaged<ViewLS<ST**,Props...>>(Kokkos::subview(v, Kokkos::ALL, kp0));
  } else {
    // FIXME: better way to do this? necessary?
    assert(false);
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
  assert(idim >= 0 && idim <= v.rank);
  // NOTE: the final comparison is originally int <= long unsigned int
  // the cast silences a warning, but may be unnecessary
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && (unsigned int)kp0.second <= v.extent(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST***,Props...>>(
      Kokkos::subview(v, kp0, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 1) {
    return Unmanaged<ViewLS<ST***,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, kp0, Kokkos::ALL));
  } else if (idim == 2) {
    return Unmanaged<ViewLS<ST***,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, kp0));
  } else {
    // FIXME: better way to do this? necessary?
    assert(false);
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
  assert(idim >= 0 && idim <= v.rank);
  // NOTE: the final comparison is originally int <= long unsigned int
  // the cast silences a warning, but may be unnecessary
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && (unsigned int)kp0.second <= v.extent(idim));
  if (idim == 0) {
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, kp0, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 1) {
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, kp0, Kokkos::ALL, Kokkos::ALL));
  } else if (idim == 2) {
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, kp0, Kokkos::ALL));
  } else if (idim == 3) {
    return Unmanaged<ViewLS<ST****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, kp0));
  } else {
    // FIXME: better way to do this? necessary?
    assert(false);
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
  assert(idim >= 0 && idim <= v.rank);
  // NOTE: the final comparison is originally int <= long unsigned int
  // the cast silences a warning, but may be unnecessary
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && (unsigned int)kp0.second <= v.extent(idim));
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
  } else if (idim == 4) {
    return Unmanaged<ViewLS<ST*****,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, kp0));
  } else {
    // FIXME: better way to do this? necessary?
    assert(false);
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
  assert(idim >= 0 && idim <= v.rank);
  // NOTE: the final comparison is originally int <= long unsigned int
  // the cast silences a warning, but may be unnecessary
  assert(kp0.first >= 0 && kp0.first < kp0.second
         && (unsigned int)kp0.second <= v.extent(idim));
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
  } else if (idim == 5) {
    return Unmanaged<ViewLS<ST******,Props...>>(
      Kokkos::subview(v, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL, Kokkos::ALL,
                      Kokkos::ALL, kp0));
  } else {
    // FIXME: better way to do this? necessary?
    assert(false);
  }
}

} // namespace ekat

#endif // EKAT_SUBVIEW_UTILS_HPP
