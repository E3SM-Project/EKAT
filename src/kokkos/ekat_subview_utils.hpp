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

namespace {
  template<class V>
  std::string get_layout(V v) {
    if constexpr (std::is_same_v<typename decltype(v)::traits::array_layout,Kokkos::LayoutRight>)
      return "LR";
    if constexpr (std::is_same_v<typename decltype(v)::traits::array_layout,Kokkos::LayoutStride>)
      return "LS";
    if constexpr (std::is_same_v<typename decltype(v)::traits::array_layout,Kokkos::LayoutLeft>)
      return "LL";

    return "ERROR!!!";
  }
}

// ================ Subviews along 2nd dimension ======================= //

// Note: if input rank>2, these subviews can retain LayoutRight.
//       However, Kokkos::subview only works if the output rank is <=2,
//       so for higher ranks, we manually build the output view
//       instead of relying on Kokkos::subview.
//       See https://github.com/kokkos/kokkos/issues/3757
//       If the input view has rank=2, then the output view MUST have
//       LayoutStride (there is no alternative).

namespace {

template<class T>
struct FixedD2Accessor {
  using offset_policy = FixedD2Accessor;
  using element_type = T;
  using reference = T&;
  using data_handle_type = T*;

  KOKKOS_INLINE_FUNCTION FixedD2Accessor(const int offset_) : offset(offset_) {}

  KOKKOS_INLINE_FUNCTION constexpr reference access(data_handle_type p, size_t i) const noexcept {
    return p[offset + i];
  }

  KOKKOS_INLINE_FUNCTION constexpr data_handle_type offset(data_handle_type p, size_t i) const noexcept {
    return p + m_offset + i
  }

private:
  int m_offset;
};
}

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

#ifdef KOKKOS_ENABLE_IMPL_VIEW_LEGACY
  using vt = Unmanaged<ViewLR<ST**,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  auto ret = Unmanaged<ViewLR<ST**,Props...>>(
      v.impl_track(),vm);

  auto sub = Kokkos::subview(v, Kokkos::ALL, i1, Kokkos::ALL);

  printf("LegacyView(%d,%d,%d): STRIDE: %d,%d,%d\n",
    v.extent_int(0), v.extent_int(1), v.extent_int(2),
    v.impl_map().stride_0(), v.impl_map().stride_1(), v.impl_map().stride_2());
  printf("ESV-%s(%d,%d): OFFSET: %ld, STRIDE: %d,%d\n", get_layout(ret).c_str(),
    ret.extent_int(0), ret.extent_int(1),
    offset, vm.stride_0(), vm.stride_1());
  printf("KSV-%s(%d,%d): OFFSET: %ld, STRIDE: %d,%d\n", get_layout(sub).c_str(),
    sub.extent_int(0), sub.extent_int(1),
    offset, sub.impl_map().stride_0(), sub.impl_map().stride_1());
#else
  auto ret = Unmanaged<ViewLR<ST**,Props...>>(v, Kokkos::ALL, i1, Kokkos::ALL);

  auto sub = Kokkos::subview(v, Kokkos::ALL, i1, Kokkos::ALL);
  printf("MDSpanView(%d,%d,%d): STRIDE: %d,%d,%d\n",
    v.extent_int(0), v.extent_int(1), v.extent_int(2),
    v.stride(0), v.stride(1), v.stride(2));
  printf("ESV-%s(%d,%d): STRIDE: %d,%d\n", get_layout(ret).c_str(),
    ret.extent_int(0), ret.extent_int(1),
    ret.stride(0), ret.stride(1));
  printf("KSV-%s(%d,%d): STRIDE: %d,%d\n", get_layout(sub).c_str(),
    sub.extent_int(0), sub.extent_int(1),
    sub.stride(0), sub.stride(1));
#endif

  return ret;
}

// --- Rank4 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST***,Props...>>
subview_1(const ViewLR<ST****,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

#ifdef KOKKOS_ENABLE_IMPL_VIEW_LEGACY
  using vt = Unmanaged<ViewLR<ST***,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2),v.extent(3));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  auto ret = Unmanaged<ViewLR<ST***,Props...>>(
      v.impl_track(),vm);

  auto sub = Kokkos::subview(v, Kokkos::ALL, i1, Kokkos::ALL, Kokkos::ALL);

  printf("View(%d,%d,%d,%d): STRIDE: %d,%d,%d,%d\n",
    v.extent_int(0), v.extent_int(1), v.extent_int(2), v.extent_int(3),
    v.impl_map().stride_0(), v.impl_map().stride_1(), v.impl_map().stride_2(), v.impl_map().stride_3());
  printf("ESV-%s(%d,%d,%d): OFFSET: %ld, STRIDE: %d,%d,%d\n", get_layout(ret).c_str(),
    ret.extent_int(0), ret.extent_int(1),ret.extent_int(2),
    offset, vm.stride_0(), vm.stride_1(), vm.stride_2());
  printf("KSV-%s(%d,%d,%d): OFFSET: %ld, STRIDE: %d,%d,%d\n", get_layout(sub).c_str(),
    sub.extent_int(0), sub.extent_int(1), sub.extent_int(2),
    offset, sub.impl_map().stride_0(), sub.impl_map().stride_1(), sub.impl_map().stride_2());
#else
  auto ret = Unmanaged<ViewLR<ST***,Props...>>();
#endif

  return ret;
}

// --- Rank5 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST****,Props...>>
subview_1(const ViewLR<ST*****,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

#ifdef KOKKOS_ENABLE_IMPL_VIEW_LEGACY
  using vt = Unmanaged<ViewLR<ST****,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0,0,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2),v.extent(3),v.extent(4));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  auto ret = Unmanaged<ViewLR<ST****,Props...>>(
      v.impl_track(),vm);
#else
  auto ret = Unmanaged<ViewLR<ST****,Props...>>();
#endif

  return ret;
}

// --- Rank6 --- //
template <typename ST, typename... Props>
KOKKOS_INLINE_FUNCTION
Unmanaged<ViewLR<ST*****,Props...>>
subview_1(const ViewLR<ST******,Props...>& v,
          const int i1) {
  assert(v.data() != nullptr);
  assert(i1>=0 && i1 < v.extent_int(1));

#ifdef KOKKOS_ENABLE_IMPL_VIEW_LEGACY
  using vt = Unmanaged<ViewLR<ST*****,Props...>>;
  // Figure out where the data starts, and create a tmp view with correct extents
  auto offset = v.impl_map().m_impl_offset(0,i1,0,0,0,0);
  auto tmp = vt(v.data()+offset,v.extent(0),v.extent(2),v.extent(3),v.extent(4),v.extent(5));

  // The view tmp has still the wrong stride_0 (the prod of the following dims).
  // Since we are keeping the first dimension, the stride is unchanged.
  auto vm = tmp.impl_map();
  vm.m_impl_offset.m_stride = v.impl_map().stride_0();
  auto ret = Unmanaged<ViewLR<ST*****,Props...>>(
      v.impl_track(),vm);
#else
  auto ret = Unmanaged<ViewLR<ST*****,Props...>>();
#endif

  return ret;
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
