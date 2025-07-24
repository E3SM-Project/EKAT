#ifndef EKAT_PACK_KOKKOS_HPP
#define EKAT_PACK_KOKKOS_HPP

#include "ekat_pack.hpp"
#include "ekat_kokkos_meta.hpp"
#include "ekat_kernel_assert.hpp"
#include "ekat_assert.hpp"

#include <vector>
#include <type_traits>

namespace ekat {

/* These functions combine Pack, Mask, and Kokkos::Views.
 */

// Index a scalar array with Pack indices, returning a compatible Pack of array
// values.
template<typename Array1, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array1::non_const_value_type, IdxPack::n> >
index (const Array1& a, const IdxPack& i0,
       typename std::enable_if<Array1::rank == 1>::type* = nullptr) {
  Pack<typename Array1::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i]);
  return p;
}

template<typename Array2, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array2::non_const_value_type, IdxPack::n> >
index (const Array2& a, const IdxPack& i0, const IdxPack& i1,
       typename std::enable_if<Array2::rank == 2>::type* = nullptr) {
  Pack<typename Array2::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i], i1[i]);
  return p;
}

template<typename Array3, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array3::non_const_value_type, IdxPack::n> >
index (const Array3& a, const IdxPack& i0, const IdxPack& i1, const IdxPack& i2,
       typename std::enable_if<Array3::rank == 3>::type* = nullptr) {
  Pack<typename Array3::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i], i1[i], i2[i]);
  return p;
}

template<typename Array4, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array4::non_const_value_type, IdxPack::n> >
index (const Array4& a, const IdxPack& i0, const IdxPack& i1, const IdxPack& i2, const IdxPack& i3,
       typename std::enable_if<Array4::rank == 4>::type* = nullptr) {
  Pack<typename Array4::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i], i1[i], i2[i], i3[i]);
  return p;
}

template<typename Array5, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array5::non_const_value_type, IdxPack::n> >
index (const Array5& a, const IdxPack& i0, const IdxPack& i1, const IdxPack& i2, const IdxPack& i3, const IdxPack& i4,
       typename std::enable_if<Array5::rank == 5>::type* = nullptr) {
  Pack<typename Array5::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i], i1[i], i2[i], i3[i], i4[i]);
  return p;
}

// Index a scalar array with Pack indices, returning a two compatible Packs of array
// values, one with the indexes shifted by Shift. This is useful for implementing
// functions like:
//   y2(k2) = y1(k1) + y1(k1+1);
// which becomes
//   index_and_shift<1>(y1, kpk, y1k, y1k1);
//   y2(k2) = y1k + y1k1
// Note: if k1+Shift is OOB, we use ekat::invalid<T>() where T is the array valye type
template<int Shift, typename Array1, typename IdxPack> KOKKOS_INLINE_FUNCTION
void
index_and_shift (const Array1& a, const IdxPack& i0, Pack<typename Array1::non_const_value_type, IdxPack::n>& index, Pack<typename Array1::non_const_value_type, IdxPack::n>& index_shift,
                 typename std::enable_if<Array1::rank == 1>::type* = nullptr) {
#ifndef NDEBUG
  // In debug, intialize index_shift to all invalids. This ensures errors will
  // happen if client tries to use values that fall outside of valid range.
  using scalar_t = typename Array1::non_const_value_type;
  index_shift = Pack<scalar_t, IdxPack::n>(invalid<scalar_t>());
#endif
  vector_simd for (int i = 0; i < IdxPack::n; ++i) {
    const auto i0i = i0[i];
#ifndef NDEBUG
    // We don't want to trigger kokkos OOB errors if they are on
    if ((i0i+Shift) < 0 || (i0i+Shift)>=static_cast<int>(a.size())) continue;
#endif
    index[i]       = a(i0i);
    index_shift[i] = a(i0i + Shift);
  }
}

// Compute the difference between adjacent entries of a view
// A few remarks:
//   - If Forward=true, returns the diff of v(index+1) - v(index),
//     otherwise returns v(index)-v(index-1)
//   - If ScalarT is a pack, the result should be interpreted as
//     a pack build from the result of computing the adj_diff of
//     a scalarized view, and then repack the answer for the current index
//   - If index+1 (for Forward=true) or index-1 (for Forward=false)
//     is OOB, the corresponding value is taken to be 0. The diff
//     is not well defined there, so the user is responsible for using
//     the returned value in a meaningful way (i.e., discard those entries)
//   - The input 'index' refers to the index space of the view. That is,
//     for ScalarT=Pack, it should be the PACK index, not the index in
//     a scalarized view.
template<bool Forward, typename ScalarT, typename... Props>
KOKKOS_INLINE_FUNCTION
typename std::enable_if<IsPack<ScalarT>::value,
                        typename std::remove_cv<ScalarT>::type>::type
adj_diff(const Kokkos::View<ScalarT*,Props...>& v, int index)
{
  ScalarT ret = v(index);
  if constexpr (Forward) {
    const int last_idx = v.size()-1;
    auto fill_right = index<last_idx ? v(index+1)[0] : 0;
    ret.update(ekat::shift_left(fill_right,ret),1,-1);
  } else {
    constexpr auto N = ScalarT::n;
    auto fill_left = index>0 ? v(index-1)[N-1] : 0;
    ret.update(ekat::shift_right(fill_left,ret),-1,1);
  }
  return ret;
}

template<bool Forward, typename ScalarT, typename... Props>
KOKKOS_INLINE_FUNCTION
typename std::enable_if<not IsPack<ScalarT>::value,
                        typename std::remove_cv<ScalarT>::type>::type
adj_diff(const Kokkos::View<ScalarT*,Props...>& v, int index)
{
  ScalarT ret;
  if constexpr (Forward) {
    const int last_idx = v.size()-1;
    if (index<last_idx)
      ret = v(index+1);
    ret -= v(index);
  } else {
    ret = v(index);
    if (index>0)
      ret -= v(index-1);
  }
  return ret;
}

// Turn a View of Packs into a View of scalars.
// Example: const auto b = scalarize(a);

// Default impl returns the input view, regardless of rank
template<typename ScalarT>
struct ScalarizeHelper
{
  template <typename ViewT>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<ViewT>
  scalarize (const ViewT& v) { return v; }
};

// Specialization if ScalarT is a non-const Pack type
template<typename T, int N>
struct ScalarizeHelper<Pack<T,N>>
{
  using PackT = Pack<T,N>;

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T****, Parms...> >
  scalarize (const Kokkos::View<PackT****, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T****, Parms...> >(
      reinterpret_cast<T*>(vp.data()), vp.extent_int(0), vp.extent_int(1),
                                       vp.extent_int(2), N * vp.extent_int(3));
  }

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T***, Parms...> >
  scalarize (const Kokkos::View<PackT***, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T***, Parms...> >(
      reinterpret_cast<T*>(vp.data()), vp.extent_int(0), vp.extent_int(1),
                                       N * vp.extent_int(2));
  }

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T**, Parms...> >
  scalarize (const Kokkos::View<PackT**, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T**, Parms...> >(
      reinterpret_cast<T*>(vp.data()), vp.extent_int(0), N * vp.extent_int(1));
  }

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T*, Parms...> >
  scalarize (const Kokkos::View<PackT*, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T*, Parms...> >(
      reinterpret_cast<T*>(vp.data()), N * vp.extent_int(0));
  }
};

// Specialization if ScalarT is a const Pack type
template<typename ScalarT, int N>
struct ScalarizeHelper<const Pack<ScalarT,N>>
{
  using PackT = const Pack<ScalarT,N>;
  using T = const ScalarT;

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T****, Parms...> >
  scalarize (const Kokkos::View<PackT****, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T****, Parms...> >(
      reinterpret_cast<T*>(vp.data()), vp.extent_int(0), vp.extent_int(1),
                                       vp.extent_int(2), N * vp.extent_int(3));
  }

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T***, Parms...> >
  scalarize (const Kokkos::View<PackT***, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T***, Parms...> >(
      reinterpret_cast<T*>(vp.data()), vp.extent_int(0), vp.extent_int(1),
                                       N * vp.extent_int(2));
  }

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T**, Parms...> >
  scalarize (const Kokkos::View<PackT**, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T**, Parms...> >(
      reinterpret_cast<T*>(vp.data()), vp.extent_int(0), N * vp.extent_int(1));
  }

  template <typename ...Parms>
  KOKKOS_FORCEINLINE_FUNCTION
  static Unmanaged<Kokkos::View<T*, Parms...> >
  scalarize (const Kokkos::View<PackT*, Parms...>& vp) {
    return Unmanaged<Kokkos::View<T*, Parms...> >(
      reinterpret_cast<T*>(vp.data()), N * vp.extent_int(0));
  }
};

// Free functions, that call the helper above
template <typename ValueT, typename ...Parms> KOKKOS_FORCEINLINE_FUNCTION
auto
scalarize (const Kokkos::View<ValueT****, Parms...>& v)
 -> decltype(ScalarizeHelper<ValueT>::scalarize(v))
{
  return ScalarizeHelper<ValueT>::scalarize(v);
}

template <typename ValueT, typename ...Parms> KOKKOS_FORCEINLINE_FUNCTION
auto
scalarize (const Kokkos::View<ValueT***, Parms...>& v)
 -> decltype(ScalarizeHelper<ValueT>::scalarize(v))
{
  return ScalarizeHelper<ValueT>::scalarize(v);
}

template <typename ValueT, typename ...Parms> KOKKOS_FORCEINLINE_FUNCTION
auto
scalarize (const Kokkos::View<ValueT**, Parms...>& v)
 -> decltype(ScalarizeHelper<ValueT>::scalarize(v))
{
  return ScalarizeHelper<ValueT>::scalarize(v);
}

template <typename ValueT, typename ...Parms> KOKKOS_FORCEINLINE_FUNCTION
auto
scalarize (const Kokkos::View<ValueT*, Parms...>& v)
 -> decltype(ScalarizeHelper<ValueT>::scalarize(v))
{
  return ScalarizeHelper<ValueT>::scalarize(v);
}

// Turn a View of Pack<T,N>s into a View of Pack<T,M>s.
// Requirement: the smaller number must divide the larger one:
//     max(M,N) % min(M,N) == 0.
// Example: const auto b = repack<4>(a);

// Helper struct
template<int N, typename OldType>
struct RepackType {
  using type =
    typename std::conditional<std::is_const<OldType>::value,
                              const Pack<std::remove_const_t<OldType>,N>,
                              Pack<OldType,N>>::type;
};

template<int N, typename T, int M>
struct RepackType <N,Pack<T,M>>{
  using type = Pack<T,N>;
};
template<int N, typename T, int M>
struct RepackType <N,const Pack<T,M>>{
  using type = const Pack<T,N>;
};

// 2d shrinking
template <typename NewPackT, typename OldPackT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
typename std::enable_if<NewPackT::packtag && OldPackT::packtag &&
    std::is_same<typename NewPackT::scalar,typename OldPackT::scalar>::value &&
    (OldPackT::n > NewPackT::n),
    Unmanaged<Kokkos::View<NewPackT**,ViewProps...>>
>::type
repack_impl (const Kokkos::View<OldPackT**, ViewProps...>& vp) {
  constexpr auto new_pack_size = NewPackT::n;
  constexpr auto old_pack_size = OldPackT::n;
  static_assert(new_pack_size > 0, "New pack size must be positive");

  // It's overly restrictive to check compatibility between pack sizes.
  // What really matters is that the new pack size divides the last extent
  // of the "scalarized" view.
  // This MUST be a runtime check.
  assert ( (vp.extent_int(1)*old_pack_size) % new_pack_size == 0);

  return Unmanaged<Kokkos::View<NewPackT**, ViewProps...> >(
    reinterpret_cast<NewPackT*>(vp.data()),
    vp.extent_int(0),
    (old_pack_size / new_pack_size) * vp.extent_int(1));
}

// 2d growing
template <typename NewPackT, typename OldPackT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
typename std::enable_if<NewPackT::packtag && OldPackT::packtag &&
    std::is_same<typename NewPackT::scalar,typename OldPackT::scalar>::value &&
    (OldPackT::n < NewPackT::n),
    Unmanaged<Kokkos::View<NewPackT**,ViewProps...>>
>::type
repack_impl (const Kokkos::View<OldPackT**, ViewProps...>& vp) {
  constexpr auto new_pack_size = NewPackT::n;
  constexpr auto old_pack_size = OldPackT::n;
  static_assert(new_pack_size > 0, "New pack size must be positive");
  // It's not enough to check that the new pack is a multiple of the old pack.
  // We actually need to check that the new pack size divides the last extent
  // of the "scalarized" view.
  // This MUST be a runtime check.
  assert ( (vp.extent_int(1)*old_pack_size) % new_pack_size == 0);

  return Unmanaged<Kokkos::View<NewPackT**, ViewProps...> >(
    reinterpret_cast<NewPackT*>(vp.data()),
    vp.extent_int(0),
    vp.extent_int(1) / (new_pack_size / old_pack_size) );
}

// 2d staying the same
template <typename NewPackT, typename OldPackT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
typename std::enable_if<NewPackT::packtag && OldPackT::packtag &&
    std::is_same<typename NewPackT::scalar,typename OldPackT::scalar>::value &&
    (OldPackT::n == NewPackT::n),
    Unmanaged<Kokkos::View<NewPackT**,ViewProps...>>
>::type
repack_impl (const Kokkos::View<OldPackT**, ViewProps...>& vp) {
  return vp;
}

// General access point for repack (calls one of the three above)
template <int N, typename OldValueT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<typename RepackType<N,OldValueT>::type**,ViewProps...>>
repack (const Kokkos::View<OldValueT**, ViewProps...>& v) {
  using OldPackT =
    typename std::conditional<IsPack<OldValueT>::value,
                              OldValueT,
                              typename RepackType<1,OldValueT>::type
                             >::type;

  // We are not changing the layout of the view, since
  //  - if OldValueT was a pack, OldPackT=OldValueT
  //  - if OldValueT was NOT a pack, OldPackT has size 1
  Kokkos::View<OldPackT**,ViewProps...> vp(
      reinterpret_cast<OldPackT*>(v.data()),v.extent(0),v.extent(1));
  return repack_impl<typename RepackType<N,OldPackT>::type>(vp);
}

// 1d shrinking
template <typename NewPackT, typename OldPackT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
typename std::enable_if<NewPackT::packtag && OldPackT::packtag &&
    std::is_same<typename NewPackT::scalar,typename OldPackT::scalar>::value &&
    (OldPackT::n > NewPackT::n),
    Unmanaged<Kokkos::View<NewPackT*,ViewProps...>>
>::type
repack_impl (const Kokkos::View<OldPackT*, ViewProps...>& vp) {
  constexpr auto new_pack_size = NewPackT::n;
  constexpr auto old_pack_size = OldPackT::n;
  static_assert(new_pack_size > 0, "New pack size must be positive");

  // It's overly restrictive to check compatibility between pack sizes.
  // What really matters is that the new pack size divides the last extent
  // of the "scalarized" view.
  // This MUST be a runtime check.
  assert ( (vp.extent_int(0)*old_pack_size) % new_pack_size == 0);

  return Unmanaged<Kokkos::View<NewPackT*, ViewProps...> >(
    reinterpret_cast<NewPackT*>(vp.data()),
    (old_pack_size / new_pack_size) * vp.extent_int(0));
}

// 1d growing
template <typename NewPackT, typename OldPackT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
typename std::enable_if<NewPackT::packtag && OldPackT::packtag &&
    std::is_same<typename NewPackT::scalar,typename OldPackT::scalar>::value &&
    (OldPackT::n < NewPackT::n),
    Unmanaged<Kokkos::View<NewPackT*,ViewProps...>>
>::type
repack_impl (const Kokkos::View<OldPackT*, ViewProps...>& vp) {
  constexpr auto new_pack_size = NewPackT::n;
  constexpr auto old_pack_size = OldPackT::n;
  static_assert(new_pack_size > 0, "New pack size must be positive");

  // It's not enough to check that the new pack is a multiple of the old pack.
  // We actually need to check that the new pack size divides the last extent
  // of the "scalarized" view.
  // This MUST be a runtime check.
  assert ( (vp.extent_int(0)*old_pack_size) % new_pack_size == 0);

  EKAT_KERNEL_ASSERT(vp.extent_int(0) % (new_pack_size / old_pack_size) == 0);
  return Unmanaged<Kokkos::View<NewPackT*, ViewProps...> >(
    reinterpret_cast<NewPackT*>(vp.data()),
    vp.extent_int(0) / (new_pack_size / old_pack_size));
}

// 1d staying the same
template <typename NewPackT, typename OldPackT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
typename std::enable_if<NewPackT::packtag && OldPackT::packtag &&
    std::is_same<typename NewPackT::scalar,typename OldPackT::scalar>::value &&
    (OldPackT::n == NewPackT::n),
    Unmanaged<Kokkos::View<NewPackT*,ViewProps...>>
>::type
repack_impl (const Kokkos::View<OldPackT*, ViewProps...>& vp) {
  return vp;
}

// General access point for repack (calls one of the three above)
template <int N, typename OldValueT, typename... ViewProps>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<typename RepackType<N,OldValueT>::type*,ViewProps...>>
repack (const Kokkos::View<OldValueT*, ViewProps...>& v) {
  using OldPackT =
    typename std::conditional<IsPack<OldValueT>::value,
                              OldValueT,
                              typename RepackType<1,OldValueT>::type
                             >::type;

  // We are not changing the layout of the view, since
  //  - if OldValueT was a pack, OldPackT=OldValueT
  //  - if OldValueT was NOT a pack, OldPackT has size 1
  Kokkos::View<OldPackT*,ViewProps...> vp(
      reinterpret_cast<OldPackT*>(v.data()),v.extent(0));
  return repack_impl<typename RepackType<N,OldPackT>::type>(vp);
}

//
// Take an array of Host scalar pointers and turn them into device pack views
//

template <typename T>
struct HTDVectorT
{ using type = T; };

template<>
struct HTDVectorT<bool>
{
  static_assert(
    sizeof(bool) == sizeof(char),
    "host_to_device/device_to_host use vectors as flexible memory buffers "
    "and they make use of vector API calls that are not available in the "
    "vector<bool> specialization, so if a user is sending bool data, we must "
    "use vector<char> instead and reinterpret the data as bool*. This is only "
    "valid if chars and bools are the same size");

  using type = char;
};

// 1d
template <typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::value_type::scalar const*>& data,
               const std::vector<SizeT>& sizes,
               std::vector<ViewT>& views)
{
  using PackT = typename ViewT::value_type;

  EKAT_ASSERT(data.size() == sizes.size());
  EKAT_ASSERT(data.size() == sizes.size());

  for (size_t i = 0; i < data.size(); ++i) {
    const size_t size = static_cast<size_t>(sizes[i]);
    const size_t npack = (size + PackT::n - 1) / PackT::n;
    views[i] = ViewT("", npack);
    auto host_view = Kokkos::create_mirror_view(views[i]);
    for (size_t k = 0; k < npack; ++k) {
      const size_t scalar_offset = k*PackT::n;
      for (size_t s = 0; s < PackT::n && scalar_offset+s < size; ++s) {
        host_view(k)[s] = data[i][scalar_offset + s];
      }
    }
    Kokkos::deep_copy(views[i], host_view);
  }
}

// 2d - set do_transpose to true if host data is coming from fortran
template <typename TransposeDirection::Enum direction = TransposeDirection::f2c,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::value_type::scalar const*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  EKAT_ASSERT(data.size() == dim1_sizes.size());
  EKAT_ASSERT(data.size() == dim2_sizes.size());
  EKAT_ASSERT(data.size() == views.size());

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < data.size(); ++n) {
    const size_t dim1_size = static_cast<size_t>(dim1_sizes[n]);
    const size_t dim2_size = static_cast<size_t>(dim2_sizes[n]);
    const size_t npack = (dim2_size + PackT::n - 1) / PackT::n;
    views[n] = ViewT("", dim1_size, npack);
    auto host_view = Kokkos::create_mirror_view(views[n]);

    ScalarT* the_data = nullptr;
    if (do_transpose) {
      tdata.reserve(dim1_size * dim2_size);
      the_data = reinterpret_cast<ScalarT*>(tdata.data());
      transpose<direction>(data[n], the_data, dim1_size, dim2_size);
    }
    else {
      the_data = const_cast<ScalarT*>(data[n]);
    }

    for (size_t i = 0; i < dim1_size; ++i) {
      for (size_t k = 0; k < npack; ++k) {
        const size_t num_scalars_this_col = k*PackT::n;
        const size_t scalar_offset = i*dim2_size + num_scalars_this_col;
        for (size_t s = 0; s < PackT::n && num_scalars_this_col+s < dim2_size; ++s) {
          host_view(i, k)[s] = the_data[scalar_offset + s];
        }
      }
    }
    Kokkos::deep_copy(views[n], host_view);
  }
}

// 3d - set do_transpose to true if host data is coming from fortran
template <typename TransposeDirection::Enum direction = TransposeDirection::f2c,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::value_type::scalar const*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               const std::vector<SizeT>& dim3_sizes,
               std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  EKAT_ASSERT(data.size() == dim1_sizes.size());
  EKAT_ASSERT(data.size() == dim2_sizes.size());
  EKAT_ASSERT(data.size() == dim3_sizes.size());
  EKAT_ASSERT(data.size() == views.size());

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < data.size(); ++n) {
    const size_t dim1_size = static_cast<size_t>(dim1_sizes[n]);
    const size_t dim2_size = static_cast<size_t>(dim2_sizes[n]);
    const size_t dim3_size = static_cast<size_t>(dim3_sizes[n]);
    const size_t npack = (dim3_size + PackT::n - 1) / PackT::n;
    views[n] = ViewT("", dim1_size, dim2_size, npack);
    auto host_view = Kokkos::create_mirror_view(views[n]);

    ScalarT* the_data = nullptr;
    if (do_transpose) {
      tdata.reserve(dim1_size * dim2_size * dim3_size);
      the_data = reinterpret_cast<ScalarT*>(tdata.data());
      transpose<direction>(data[n], the_data, dim1_size, dim2_size, dim3_size);
    }
    else {
      the_data = const_cast<ScalarT*>(data[n]);
    }

    for (size_t i = 0; i < dim1_size; ++i) {
      for (size_t k = 0; k < dim2_size; ++k) {
        for (size_t p = 0; p < npack; ++p) {
          const size_t num_scalars_this_col = p*PackT::n;
          const size_t scalar_offset = i*(dim2_size*dim3_size) + k*dim3_size + num_scalars_this_col;
          for (size_t s = 0; s < PackT::n && num_scalars_this_col+s < dim3_size; ++s) {
            host_view(i, k, p)[s] = the_data[scalar_offset + s];
          }
        }
      }
    }
    Kokkos::deep_copy(views[n], host_view);
  }
}

// Sugar for when size is uniform (1d)
template <typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::value_type::scalar const*>& data,
                    const SizeT size,
                    std::vector<ViewT>& views)
{
  std::vector<SizeT> sizes(data.size(), size);
  host_to_device(data, sizes, views);
}

// Sugar for when size is uniform (2d)
template <typename TransposeDirection::Enum direction = TransposeDirection::f2c,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::value_type::scalar const*>& data,
               const SizeT dim1_size, const SizeT dim2_size,
               std::vector<ViewT>& views,
               bool do_transpose=false)
{
  std::vector<SizeT> dim1_sizes(data.size(), dim1_size), dim2_sizes(data.size(), dim2_size);
  host_to_device<direction>(data, dim1_sizes, dim2_sizes, views, do_transpose);
}

// Sugar for when size is uniform (3d)
template <typename TransposeDirection::Enum direction = TransposeDirection::f2c,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::value_type::scalar const*>& data,
               const SizeT dim1_size, const SizeT dim2_size, const SizeT dim3_size,
               std::vector<ViewT>& views,
               bool do_transpose=false)
{
  std::vector<SizeT> dim1_sizes(data.size(), dim1_size), dim2_sizes(data.size(), dim2_size), dim3_sizes(data.size(), dim3_size);
  host_to_device<direction>(data, dim1_sizes, dim2_sizes, dim3_sizes, views, do_transpose);
}

// Sugar for unpacked data
template <typename SizeT, typename ViewT>
typename std::enable_if<!IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::const_value_type*>& data,
               const std::vector<SizeT>& sizes,
               std::vector<ViewT>& views)
{
  using ScalarT = typename ViewT::value_type;
  using PackT = Pack<ScalarT, 1>;
  using ViewFakePackT = Kokkos::View<PackT*, typename ViewT::array_layout, typename ViewT::memory_space, typename ViewT::memory_traits>;

  std::vector<ViewFakePackT>& views_fake = reinterpret_cast<std::vector<ViewFakePackT>&>(views);
  host_to_device(data, sizes, views_fake);
}

// Sugar for unpacked data
template <typename TransposeDirection::Enum direction = TransposeDirection::f2c,
          typename SizeT, typename ViewT>
typename std::enable_if<!IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::const_value_type*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using ScalarT = typename ViewT::value_type;
  using PackT = Pack<ScalarT, 1>;
  using ViewFakePackT = Kokkos::View<PackT**, typename ViewT::array_layout, typename ViewT::memory_space, typename ViewT::memory_traits>;

  std::vector<ViewFakePackT>& views_fake = reinterpret_cast<std::vector<ViewFakePackT>&>(views);
  host_to_device<direction>(data, dim1_sizes, dim2_sizes, views_fake, do_transpose);
}

// Sugar for unpacked data
template <typename TransposeDirection::Enum direction = TransposeDirection::f2c,
          typename SizeT, typename ViewT>
typename std::enable_if<!IsPack<typename ViewT::value_type>::value, void>::type // void
host_to_device(const std::vector<typename ViewT::const_value_type*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               const std::vector<SizeT>& dim3_sizes,
               std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using ScalarT = typename ViewT::value_type;
  using PackT = Pack<ScalarT, 1>;
  using ViewFakePackT = Kokkos::View<PackT***, typename ViewT::array_layout, typename ViewT::memory_space, typename ViewT::memory_traits>;

  std::vector<ViewFakePackT>& views_fake = reinterpret_cast<std::vector<ViewFakePackT>&>(views);
  host_to_device<direction>(data, dim1_sizes, dim2_sizes, dim3_sizes, views_fake, do_transpose);
}

//
// Take an array of device pack views and sync them to host scalar pointers
//

// 1d
template <typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type::scalar*>& data,
               const std::vector<SizeT>& sizes,
               const std::vector<ViewT>& views)
{
  using PackT = typename ViewT::value_type;

  EKAT_ASSERT(data.size() == sizes.size());
  EKAT_ASSERT(data.size() == sizes.size());

  for (size_t i = 0; i < data.size(); ++i) {
    const size_t size = static_cast<size_t>(sizes[i]);
    const auto host_view = Kokkos::create_mirror_view(views[i]);
    Kokkos::deep_copy(host_view, views[i]);
    for (size_t k = 0; k < views[i].extent(0); ++k) {
      const size_t scalar_offset = k*PackT::n;
      for (size_t s = 0; s < PackT::n && scalar_offset+s < size; ++s) {
        data[i][scalar_offset + s] = host_view(k)[s];
      }
    }
  }
}

// 2d - set do_transpose to true if host data is going to fortran
template <typename TransposeDirection::Enum direction = TransposeDirection::c2f,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type::scalar*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               const std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  EKAT_ASSERT(data.size() == dim1_sizes.size());
  EKAT_ASSERT(data.size() == dim2_sizes.size());
  EKAT_ASSERT(data.size() == views.size());

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < data.size(); ++n) {
    const size_t dim1_size = static_cast<size_t>(dim1_sizes[n]);
    const size_t dim2_size = static_cast<size_t>(dim2_sizes[n]);
    const size_t npack = views[n].extent(1);
    const auto host_view = Kokkos::create_mirror_view(views[n]);
    Kokkos::deep_copy(host_view, views[n]);

    ScalarT* the_data = nullptr;
    if (do_transpose) {
      tdata.reserve(dim1_size * dim2_size);
      the_data = reinterpret_cast<ScalarT*>(tdata.data());
    }
    else {
      the_data = data[n];
    }

    for (size_t i = 0; i < dim1_size; ++i) {
      for (size_t k = 0; k < npack; ++k) {
        const size_t num_scalars_this_col = k*PackT::n;
        const size_t scalar_offset = i*dim2_size + num_scalars_this_col;
        for (size_t s = 0; s < PackT::n && num_scalars_this_col+s < dim2_size; ++s) {
          the_data[scalar_offset + s] = host_view(i, k)[s];
        }
      }
    }

    if (do_transpose) {
      transpose<direction>(the_data, data[n], dim1_size, dim2_size);
    }
  }
}

// 3d - set do_transpose to true if host data is going to fortran
template <typename TransposeDirection::Enum direction = TransposeDirection::c2f,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type::scalar*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               const std::vector<SizeT>& dim3_sizes,
               const std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  EKAT_ASSERT(data.size() == dim1_sizes.size());
  EKAT_ASSERT(data.size() == dim2_sizes.size());
  EKAT_ASSERT(data.size() == dim3_sizes.size());
  EKAT_ASSERT(data.size() == views.size());

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < data.size(); ++n) {
    const size_t dim1_size = static_cast<size_t>(dim1_sizes[n]);
    const size_t dim2_size = static_cast<size_t>(dim2_sizes[n]);
    const size_t dim3_size = static_cast<size_t>(dim3_sizes[n]);
    const size_t npack = views[n].extent(2);
    const auto host_view = Kokkos::create_mirror_view(views[n]);
    Kokkos::deep_copy(host_view, views[n]);

    ScalarT* the_data = nullptr;
    if (do_transpose) {
      tdata.reserve(dim1_size * dim2_size * dim3_size);
      the_data = reinterpret_cast<ScalarT*>(tdata.data());
    }
    else {
      the_data = data[n];
    }

    for (size_t i = 0; i < dim1_size; ++i) {
      for (size_t k = 0; k < dim2_size; ++k) {
        for (size_t p = 0; p < npack; ++p) {
          const size_t num_scalars_this_col = p*PackT::n;
          const size_t scalar_offset = i*(dim2_size*dim3_size) + k*dim3_size + num_scalars_this_col;
          for (size_t s = 0; s < PackT::n && num_scalars_this_col+s < dim3_size; ++s) {
            the_data[scalar_offset + s] = host_view(i, k, p)[s];
          }
        }
      }
    }

    if (do_transpose) {
      transpose<direction>(the_data, data[n], dim1_size, dim2_size, dim3_size);
    }
  }
}

// Sugar for when size is uniform (1d)
template <typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type::scalar*>& data,
               const SizeT size,
               const std::vector<ViewT>& views)
{
  std::vector<SizeT> sizes(data.size(), size);
  device_to_host(data, sizes, views);
}

// Sugar for when size is uniform (2d)
template <typename TransposeDirection::Enum direction = TransposeDirection::c2f,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type::scalar*>& data,
               const SizeT dim1_size, const SizeT dim2_size,
               const std::vector<ViewT>& views,
               bool do_transpose=false)
{
  std::vector<SizeT> dim1_sizes(data.size(), dim1_size), dim2_sizes(data.size(), dim2_size);
  device_to_host<direction>(data, dim1_sizes, dim2_sizes, views, do_transpose);
}

// Sugar for when size is uniform (3d)
template <typename TransposeDirection::Enum direction = TransposeDirection::c2f,
          typename SizeT, typename ViewT>
typename std::enable_if<IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type::scalar*>& data,
               const SizeT dim1_size, const SizeT dim2_size, const SizeT dim3_size,
               const std::vector<ViewT>& views,
               bool do_transpose=false)
{
  std::vector<SizeT> dim1_sizes(data.size(), dim1_size), dim2_sizes(data.size(), dim2_size), dim3_sizes(data.size(), dim3_size);
  device_to_host<direction>(data, dim1_sizes, dim2_sizes, dim3_sizes, views, do_transpose);
}

// Sugar for unpacked data
template <typename SizeT, typename ViewT>
typename std::enable_if<!IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type*>& data,
               const std::vector<SizeT>& sizes,
               const std::vector<ViewT>& views)
{
  using ScalarT = typename ViewT::value_type;
  using PackT = Pack<ScalarT, 1>;
  using ViewFakePackT = Kokkos::View<PackT*, typename ViewT::array_layout, typename ViewT::memory_space, typename ViewT::memory_traits>;

  const std::vector<ViewFakePackT>& views_fake = reinterpret_cast<const std::vector<ViewFakePackT>&>(views);
  device_to_host(data, sizes, views_fake);
}

// Sugar for unpacked data
template <typename TransposeDirection::Enum direction = TransposeDirection::c2f,
          typename SizeT, typename ViewT>
typename std::enable_if<!IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               const std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using ScalarT = typename ViewT::value_type;
  using PackT = Pack<ScalarT, 1>;
  using ViewFakePackT = Kokkos::View<PackT**, typename ViewT::array_layout, typename ViewT::memory_space, typename ViewT::memory_traits>;

  const std::vector<ViewFakePackT>& views_fake = reinterpret_cast<const std::vector<ViewFakePackT>&>(views);
  device_to_host<direction>(data, dim1_sizes, dim2_sizes, views_fake, do_transpose);
}

// Sugar for unpacked data
template <typename TransposeDirection::Enum direction = TransposeDirection::c2f,
          typename SizeT, typename ViewT>
typename std::enable_if<!IsPack<typename ViewT::value_type>::value, void>::type // void
device_to_host(const std::vector<typename ViewT::non_const_value_type*>& data,
               const std::vector<SizeT>& dim1_sizes,
               const std::vector<SizeT>& dim2_sizes,
               const std::vector<SizeT>& dim3_sizes,
               std::vector<ViewT>& views,
               bool do_transpose=false)
{
  using ScalarT = typename ViewT::value_type;
  using PackT = Pack<ScalarT, 1>;
  using ViewFakePackT = Kokkos::View<PackT***, typename ViewT::array_layout, typename ViewT::memory_space, typename ViewT::memory_traits>;

  const std::vector<ViewFakePackT>& views_fake = reinterpret_cast<const std::vector<ViewFakePackT>&>(views);
  device_to_host<direction>(data, dim1_sizes, dim2_sizes, dim3_sizes, views_fake, do_transpose);
}

} // namespace ekat

#endif // EKAT_PACK_KOKKOS_HPP
