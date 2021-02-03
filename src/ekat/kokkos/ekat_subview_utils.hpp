#ifndef EKAT_SUBVIEW_UTILS_HPP
#define EKAT_SUBVIEW_UTILS_HPP

#include "ekat/kokkos/ekat_kokkos_types.hpp"
#include "ekat/kokkos/ekat_kokkos_meta.hpp"

namespace ekat {

// ================ Subviews of several ranks views ======================= //

// Note: we template on scalar type ST to allow both builtin and Packs,
//       as well as to allow const/non-const versions.

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

// Note: these subviews can retain LayoutRight, with a single stride
//       However, Kokkos::subview only works if the output rank is <=2,
//       so for higher ranks, we manually build the output view
//       instead of relying on Kokkos::subview.
//       See https://github.com/kokkos/kokkos/issues/3757
// Note: we *cannot* offer this method for Rank2 views, since
//       kokkos does not offer LayoutRight with a stride for dim0
//       for rank 1 layouts, and forces to use LayoutStride instead.
//       That means that v(m,n) subviewed as v(:,n0) will *always*
//       have LayoutStride, which we try to avoid.

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
  auto test =  Unmanaged<ViewLR<ST***,Props...>>(v.impl_track(),vm);
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

} // namespace ekat

#endif // EKAT_SUBVIEW_UTILS_HPP
