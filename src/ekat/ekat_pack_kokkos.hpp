#ifndef EKAT_PACK_KOKKOS_HPP
#define EKAT_PACK_KOKKOS_HPP

#include "ekat/ekat_pack.hpp"
#include "ekat/kokkos/ekat_kokkos_utils.hpp"
#include "ekat/ekat.hpp"

#include <Kokkos_Core.hpp>

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
       typename std::enable_if<Array1::Rank == 1>::type* = nullptr) {
  Pack<typename Array1::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i]);
  return p;
}

template<typename Array2, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array2::non_const_value_type, IdxPack::n> >
index (const Array2& a, const IdxPack& i0, const IdxPack& i1,
       typename std::enable_if<Array2::Rank == 2>::type* = nullptr) {
  Pack<typename Array2::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i], i1[i]);
  return p;
}

template<typename Array3, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array3::non_const_value_type, IdxPack::n> >
index (const Array3& a, const IdxPack& i0, const IdxPack& i1, const IdxPack& i2,
       typename std::enable_if<Array3::Rank == 3>::type* = nullptr) {
  Pack<typename Array3::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i], i1[i], i2[i]);
  return p;
}

template<typename Array4, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array4::non_const_value_type, IdxPack::n> >
index (const Array4& a, const IdxPack& i0, const IdxPack& i1, const IdxPack& i2, const IdxPack& i3,
       typename std::enable_if<Array4::Rank == 4>::type* = nullptr) {
  Pack<typename Array4::non_const_value_type, IdxPack::n> p;
  vector_simd for (int i = 0; i < IdxPack::n; ++i)
    p[i] = a(i0[i], i1[i], i2[i], i3[i]);
  return p;
}

template<typename Array5, typename IdxPack> KOKKOS_INLINE_FUNCTION
OnlyPackReturn<IdxPack, Pack<typename Array5::non_const_value_type, IdxPack::n> >
index (const Array5& a, const IdxPack& i0, const IdxPack& i1, const IdxPack& i2, const IdxPack& i3, const IdxPack& i4,
       typename std::enable_if<Array5::Rank == 5>::type* = nullptr) {
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
template<int Shift, typename Array1, typename IdxPack> KOKKOS_INLINE_FUNCTION
void
index_and_shift (const Array1& a, const IdxPack& i0, Pack<typename Array1::non_const_value_type, IdxPack::n>& index, Pack<typename Array1::non_const_value_type, IdxPack::n>& index_shift,
                 typename std::enable_if<Array1::Rank == 1>::type* = nullptr) {
  vector_simd for (int i = 0; i < IdxPack::n; ++i) {
    const auto i0i = i0[i];
    index[i]       = a(i0i);
    index_shift[i] = a(i0i + Shift);
  }
}

// Turn a View of Packs into a View of scalars.
// Example: const auto b = scalarize(a);

// 4d
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<T****, Parms...> >
scalarize (const Kokkos::View<Pack<T, pack_size>****, Parms...>& vp) {
  return Unmanaged<Kokkos::View<T****, Parms...> >(
    reinterpret_cast<T*>(vp.data()),
    vp.extent_int(0), vp.extent_int(1), vp.extent_int(2),
    pack_size * vp.extent_int(3));
}

// 4d const
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<const T****, Parms...> >
scalarize (const Kokkos::View<const Pack<T, pack_size>****, Parms...>& vp) {
  return Unmanaged<Kokkos::View<const T****, Parms...> >(
    reinterpret_cast<const T*>(vp.data()),
    vp.extent_int(0), vp.extent_int(1), vp.extent_int(2),
    pack_size * vp.extent_int(3));
}

// 3d
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<T***, Parms...> >
scalarize (const Kokkos::View<Pack<T, pack_size>***, Parms...>& vp) {
  return Unmanaged<Kokkos::View<T***, Parms...> >(
    reinterpret_cast<T*>(vp.data()), vp.extent_int(0), vp.extent_int(1),
    pack_size * vp.extent_int(2));
}

// 3d const
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<const T***, Parms...> >
scalarize (const Kokkos::View<const Pack<T, pack_size>***, Parms...>& vp) {
  return Unmanaged<Kokkos::View<const T***, Parms...> >(
    reinterpret_cast<const T*>(vp.data()), vp.extent_int(0), vp.extent_int(1),
    pack_size * vp.extent_int(2));
}

// 2d
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<T**, Parms...> >
scalarize (const Kokkos::View<Pack<T, pack_size>**, Parms...>& vp) {
  return Unmanaged<Kokkos::View<T**, Parms...> >(
    reinterpret_cast<T*>(vp.data()), vp.extent_int(0), pack_size * vp.extent_int(1));
}

// 2d const
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<const T**, Parms...> >
scalarize (const Kokkos::View<const Pack<T, pack_size>**, Parms...>& vp) {
  return Unmanaged<Kokkos::View<const T**, Parms...> >(
    reinterpret_cast<const T*>(vp.data()), vp.extent_int(0), pack_size * vp.extent_int(1));
}

// 1d
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<T*, Parms...> >
scalarize (const Kokkos::View<Pack<T, pack_size>*, Parms...>& vp) {
  return Unmanaged<Kokkos::View<T*, Parms...> >(
    reinterpret_cast<T*>(vp.data()), pack_size * vp.extent_int(0));
}

// 1d const
template <typename T, typename ...Parms, int pack_size> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<const T*, Parms...> >
scalarize (const Kokkos::View<const Pack<T, pack_size>*, Parms...>& vp) {
  return Unmanaged<Kokkos::View<const T*, Parms...> >(
    reinterpret_cast<const T*>(vp.data()), pack_size * vp.extent_int(0));
}

// Turn a View of Pack<T,N>s into a View of Pack<T,M>s. M must divide N:
//     N % M == 0.
// Example: const auto b = repack<4>(a);

// 2d shrinking
template <int new_pack_size,
          typename T, typename ...Parms, int old_pack_size,
          typename std::enable_if<(old_pack_size > new_pack_size), int>::type = 0>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<Pack<T, new_pack_size>**, Parms...> >
repack (const Kokkos::View<Pack<T, old_pack_size>**, Parms...>& vp) {
  static_assert(new_pack_size > 0 &&
                old_pack_size % new_pack_size == 0,
                "New pack size must divide old pack size.");
  return Unmanaged<Kokkos::View<Pack<T, new_pack_size>**, Parms...> >(
    reinterpret_cast<Pack<T, new_pack_size>*>(vp.data()),
    vp.extent_int(0),
    (old_pack_size / new_pack_size) * vp.extent_int(1));
}

// 2d growing
template <int new_pack_size,
          typename T, typename ...Parms, int old_pack_size,
          typename std::enable_if<(old_pack_size < new_pack_size), int>::type = 0>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<Pack<T, new_pack_size>**, Parms...> >
repack (const Kokkos::View<Pack<T, old_pack_size>**, Parms...>& vp) {
  static_assert(new_pack_size % old_pack_size == 0,
                "New pack size must divide old pack size.");
  return Unmanaged<Kokkos::View<Pack<T, new_pack_size>**, Parms...> >(
    reinterpret_cast<Pack<T, new_pack_size>*>(vp.data()),
    vp.extent_int(0),
    (new_pack_size / old_pack_size) * vp.extent_int(1));
}

// 2d staying the same
template <int new_pack_size,
          typename T, typename ...Parms, int old_pack_size,
          typename std::enable_if<(new_pack_size == old_pack_size), int>::type = 0>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<Pack<T, new_pack_size>**, Parms...> >
repack (const Kokkos::View<Pack<T, old_pack_size>**, Parms...>& vp) {
  return vp;
}

// 1d shrinking
template <int new_pack_size,
          typename T, typename ...Parms, int old_pack_size,
          typename std::enable_if<(old_pack_size > new_pack_size), int>::type = 0>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<Pack<T, new_pack_size>*, Parms...> >
repack (const Kokkos::View<Pack<T, old_pack_size>*, Parms...>& vp) {
  static_assert(new_pack_size > 0 &&
                old_pack_size % new_pack_size == 0,
                "New pack size must divide old pack size.");
  return Unmanaged<Kokkos::View<Pack<T, new_pack_size>*, Parms...> >(
    reinterpret_cast<Pack<T, new_pack_size>*>(vp.data()),
    (old_pack_size / new_pack_size) * vp.extent_int(0));
}

// 1d growing
template <int new_pack_size,
          typename T, typename ...Parms, int old_pack_size,
          typename std::enable_if<(old_pack_size < new_pack_size), int>::type = 0>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<Pack<T, new_pack_size>*, Parms...> >
repack (const Kokkos::View<Pack<T, old_pack_size>*, Parms...>& vp) {
  static_assert(new_pack_size > 0 &&
                new_pack_size % old_pack_size == 0,
                "Old pack size must divide new pack size.");
  EKAT_KERNEL_ASSERT(vp.extent_int(0) % (new_pack_size / old_pack_size) == 0);
  return Unmanaged<Kokkos::View<Pack<T, new_pack_size>*, Parms...> >(
    reinterpret_cast<Pack<T, new_pack_size>*>(vp.data()),
    vp.extent_int(0) / (new_pack_size / old_pack_size));
}

// 1d staying the same
template <int new_pack_size,
          typename T, typename ...Parms, int old_pack_size,
          typename std::enable_if<(old_pack_size == new_pack_size), int>::type = 0>
KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<Pack<T, new_pack_size>*, Parms...> >
repack (const Kokkos::View<Pack<T, old_pack_size>*, Parms...>& vp) {
  return vp;
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
  static_assert(sizeof(bool) == sizeof(char));
  using type = char;
};

// 1d
template <typename SizeT, size_t N, typename ViewT>
void host_to_device(const Kokkos::Array<typename ViewT::value_type::scalar const*, N>& data,
                    const Kokkos::Array<SizeT, N>& sizes,
                    Kokkos::Array<ViewT, N>& views)
{
  using PackT = typename ViewT::value_type;

  for (size_t i = 0; i < N; ++i) {
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
template <typename SizeT, size_t N, typename ViewT>
void host_to_device(const Kokkos::Array<typename ViewT::value_type::scalar const*, N>& data,
                    const Kokkos::Array<SizeT, N>& dim1_sizes,
                    const Kokkos::Array<SizeT, N>& dim2_sizes,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < N; ++n) {
    const size_t dim1_size = static_cast<size_t>(dim1_sizes[n]);
    const size_t dim2_size = static_cast<size_t>(dim2_sizes[n]);
    const size_t npack = (dim2_size + PackT::n - 1) / PackT::n;
    views[n] = ViewT("", dim1_size, npack);
    auto host_view = Kokkos::create_mirror_view(views[n]);

    ScalarT* the_data = nullptr;
    if (do_transpose) {
      tdata.reserve(dim1_size * dim2_size);
      the_data = reinterpret_cast<ScalarT*>(tdata.data());
      transpose<TransposeDirection::f2c>(data[n], the_data, dim1_size, dim2_size);
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
template <typename SizeT, size_t N, typename ViewT>
void host_to_device(const Kokkos::Array<typename ViewT::value_type::scalar const*, N>& data,
                    const Kokkos::Array<SizeT, N>& dim1_sizes,
                    const Kokkos::Array<SizeT, N>& dim2_sizes,
                    const Kokkos::Array<SizeT, N>& dim3_sizes,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < N; ++n) {
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
      transpose<TransposeDirection::f2c>(data[n], the_data, dim1_size, dim2_size, dim3_size);
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
template <typename SizeT, size_t N, typename ViewT>
void host_to_device(const Kokkos::Array<typename ViewT::value_type::scalar const*, N>& data,
                    const SizeT size,
                    Kokkos::Array<ViewT, N>& views)
{
  Kokkos::Array<SizeT, N> sizes;
  for (size_t i = 0; i < N; ++i) {
    sizes[i] = size;
  }
  host_to_device(data, sizes, views);
}

// Sugar for when size is uniform (2d)
template <typename SizeT, size_t N, typename ViewT>
void host_to_device(const Kokkos::Array<typename ViewT::value_type::scalar const*, N>& data,
                    const SizeT dim1_size, const SizeT dim2_size,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  Kokkos::Array<SizeT, N> dim1_sizes, dim2_sizes;
  for (size_t i = 0; i < N; ++i) {
    dim1_sizes[i] = dim1_size;
    dim2_sizes[i] = dim2_size;
  }
  host_to_device(data, dim1_sizes, dim2_sizes, views, do_transpose);
}

// Sugar for when size is uniform (3d)
template <typename SizeT, size_t N, typename ViewT>
void host_to_device(const Kokkos::Array<typename ViewT::value_type::scalar const*, N>& data,
                    const SizeT dim1_size, const SizeT dim2_size, const SizeT dim3_size,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  Kokkos::Array<SizeT, N> dim1_sizes, dim2_sizes, dim3_sizes;
  for (size_t i = 0; i < N; ++i) {
    dim1_sizes[i] = dim1_size;
    dim2_sizes[i] = dim2_size;
    dim3_sizes[i] = dim3_size;
  }
  host_to_device(data, dim1_sizes, dim2_sizes, dim3_sizes, views, do_transpose);
}

//
// Take an array of device pack views and sync them to host scalar pointers
//

// 1d
template <typename SizeT, size_t N, typename ViewT>
void device_to_host(const Kokkos::Array<typename ViewT::value_type::scalar*, N>& data,
                    const Kokkos::Array<SizeT, N>& sizes,
                    Kokkos::Array<ViewT, N>& views)
{
  using PackT = typename ViewT::value_type;

  for (size_t i = 0; i < N; ++i) {
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
template <typename SizeT, size_t N, typename ViewT>
void device_to_host(const Kokkos::Array<typename ViewT::value_type::scalar*, N>& data,
                    const Kokkos::Array<SizeT, N>& dim1_sizes,
                    const Kokkos::Array<SizeT, N>& dim2_sizes,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < N; ++n) {
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
      transpose<TransposeDirection::c2f>(the_data, data[n], dim1_size, dim2_size);
    }
  }
}

// 3d - set do_transpose to true if host data is going to fortran
template <typename SizeT, size_t N, typename ViewT>
void device_to_host(const Kokkos::Array<typename ViewT::value_type::scalar*, N>& data,
                    const Kokkos::Array<SizeT, N>& dim1_sizes,
                    const Kokkos::Array<SizeT, N>& dim2_sizes,
                    const Kokkos::Array<SizeT, N>& dim3_sizes,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  using PackT = typename ViewT::value_type;
  using ScalarT = typename PackT::scalar;
  using VectorT = typename HTDVectorT<ScalarT>::type;

  std::vector<VectorT> tdata;
  for (size_t n = 0; n < N; ++n) {
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
      transpose<TransposeDirection::c2f>(the_data, data[n], dim1_size, dim2_size, dim3_size);
    }
  }
}

// Sugar for when size is uniform (1d)
template <typename SizeT, size_t N, typename ViewT>
void device_to_host(const Kokkos::Array<typename ViewT::value_type::scalar*, N>& data,
                    const SizeT size,
                    Kokkos::Array<ViewT, N>& views)
{
  Kokkos::Array<SizeT, N> sizes;
  for (size_t i = 0; i < N; ++i) {
    sizes[i] = size;
  }
  device_to_host(data, sizes, views);
}

// Sugar for when size is uniform (2d)
template <typename SizeT, size_t N, typename ViewT>
void device_to_host(const Kokkos::Array<typename ViewT::value_type::scalar*, N>& data,
                    const SizeT dim1_size, const SizeT dim2_size,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  Kokkos::Array<SizeT, N> dim1_sizes, dim2_sizes;
  for (size_t i = 0; i < N; ++i) {
    dim1_sizes[i] = dim1_size;
    dim2_sizes[i] = dim2_size;
  }
  device_to_host(data, dim1_sizes, dim2_sizes, views, do_transpose);
}

// Sugar for when size is uniform (3d)
template <typename SizeT, size_t N, typename ViewT>
void device_to_host(const Kokkos::Array<typename ViewT::value_type::scalar*, N>& data,
                    const SizeT dim1_size, const SizeT dim2_size, const SizeT dim3_size,
                    Kokkos::Array<ViewT, N>& views,
                    bool do_transpose=false)
{
  Kokkos::Array<SizeT, N> dim1_sizes, dim2_sizes, dim3_sizes;
  for (size_t i = 0; i < N; ++i) {
    dim1_sizes[i] = dim1_size;
    dim2_sizes[i] = dim2_size;
    dim3_sizes[i] = dim3_size;
  }
  device_to_host(data, dim1_sizes, dim2_sizes, dim3_sizes, views, do_transpose);
}

} // namespace ekat

#endif // EKAT_PACK_KOKKOS_HPP
