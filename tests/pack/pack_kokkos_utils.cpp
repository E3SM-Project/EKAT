#include <catch2/catch.hpp>

#include "ekat_pack_kokkos.hpp"
#include "ekat_team_policy_utils.hpp"
#include "ekat_view_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include "ekat_test_config.h"

#include <vector>

namespace {

template <typename View, int rank, typename T = void>
using OnlyRank = typename std::enable_if<View::rank == rank, T>::type;

template <typename View>
void fill(const View& a)
{
  const auto m = Kokkos::create_mirror_view(a);
  int span = m.span();
  auto raw = m.data();
  for (int i = 0; i < span; ++i) raw[i] = i;
  Kokkos::deep_copy(a, m);
}

template <typename VA, typename VB>
void compare (const VA& a, const VB& b) {
  const auto ma = Kokkos::create_mirror_view(a);
  const auto mb = Kokkos::create_mirror_view(b);
  Kokkos::deep_copy(ma, a);
  Kokkos::deep_copy(mb, b);
  int spana = ma.span(), spanb = mb.span();
  auto rawa = ma.data(); auto rawb = mb.data();
  REQUIRE(spana == spanb);
  for (int i = 0; i < spana; ++i) REQUIRE(rawa[i] == rawb[i]);
}

template <int N, typename VA, typename VB>
void compare_s_vs_p (const VA& a, const VB& b) {
  const auto ma = Kokkos::create_mirror_view(a);
  const auto mb = Kokkos::create_mirror_view(b);
  Kokkos::deep_copy(ma, a);
  Kokkos::deep_copy(mb, b);
  int lena = a.size();
  int lenb = b.size();
  REQUIRE (lena==(lenb*N));
  auto rawa = ma.data(); auto rawb = mb.data();
  for (int i=0; i<lenb; ++i) {
    for (int j=0; j<N; ++j) {
      REQUIRE(rawa[i*N+j] == rawb[i][j]);
    }
  }
}

template <typename VA, typename VB>
void compare_p_vs_p (const VA& a, const VB& b) {
  const auto ma = Kokkos::create_mirror_view(a);
  const auto mb = Kokkos::create_mirror_view(b);
  Kokkos::deep_copy(ma, a);
  Kokkos::deep_copy(mb, b);
  int spana = ma.span(), spanb = mb.span();
  auto rawa = ma.data(); auto rawb = mb.data();
  REQUIRE(spana == spanb);
  for (int i = 0; i < spana; ++i) REQUIRE( (rawa[i] == rawb[i]).all() );
}

#define make_get_index(rank, ...)                                         \
template<typename View, typename IdxView, OnlyRank<View, rank, int> = 0 > \
KOKKOS_INLINE_FUNCTION                                                    \
ekat::Pack<typename View::value_type, IdxView::value_type::n> get_index(const View& data, const IdxView& idx) { return index(data, __VA_ARGS__); }

#define make_get_data(rank, ...)                                        \
template<typename View, typename IdxView, OnlyRank<View, rank, int> = 0 > \
KOKKOS_INLINE_FUNCTION                                                  \
typename View::value_type get_data(const View& data, const IdxView& idx, int slot) { return data(__VA_ARGS__); }

make_get_index(1, idx(0))
make_get_data(1, idx(0)[slot])
make_get_index(2, idx(0), idx(1))
make_get_data(2, idx(0)[slot], idx(1)[slot])
make_get_index(3, idx(0), idx(1), idx(2))
make_get_data(3, idx(0)[slot], idx(1)[slot], idx(2)[slot])
make_get_index(4, idx(0), idx(1), idx(2), idx(3))
make_get_data(4, idx(0)[slot], idx(1)[slot], idx(2)[slot], idx(3)[slot])
make_get_index(5, idx(0), idx(1), idx(2), idx(3), idx(4))
make_get_data(5, idx(0)[slot], idx(1)[slot], idx(2)[slot], idx(3)[slot], idx(4)[slot])

template<int Packn, typename View>
void do_index_test(const View& data)
{
  static constexpr int pack_size = Packn;
  using IdxPack = ekat::Pack<int, pack_size>;
  fill(data);
  Kokkos::View<IdxPack[View::rank]> idx("idx");
  Kokkos::parallel_for(View::rank, KOKKOS_LAMBDA(const int r) {
    for (int i = 0; i < pack_size; ++i) { idx(r)[i] = (r+2)*i; } // 2*i, 3*i, etc as rank increases
  });

  int nerr = 0;
  Kokkos::parallel_reduce(
    1, KOKKOS_LAMBDA (const int /* unused */, int& nerr) {
      const auto data_idx = get_index(data, idx);
      for (int i = 0; i < pack_size; ++i)
        if (data_idx[i] != get_data(data, idx, i))
          ++nerr;
    },
    nerr);
  REQUIRE(nerr == 0);
}

TEST_CASE("index", "ekat::pack") {
  {
    Kokkos::View<double*> data("data", 100);
    do_index_test<16>(data);
  }

  {
    Kokkos::View<double**> data("data", 19, 24);
    do_index_test<8>(data);
  }

  {
    Kokkos::View<double***> data("data", 9, 13, 17);
    do_index_test<4>(data);
  }

  {
    Kokkos::View<double****> data("data", 5, 10, 10, 15);
    do_index_test<2>(data);
  }

  {
    Kokkos::View<double*****> data("data", 5, 5, 5, 10, 10);
    do_index_test<1>(data);
  }
}

TEST_CASE("scalarize", "ekat::pack") {
  using ekat::Pack;
  using ekat::scalarize;

  typedef Kokkos::View<Pack<double, 16>*>    Array1;
  typedef Kokkos::View<Pack<double, 32>**>   Array2;
  typedef Kokkos::View<Pack<double, 8>***>   Array3;
  typedef Kokkos::View<Pack<double, 24>****> Array4;

  {
    const Array1 a1("a1", 10);
    const auto a2 = scalarize(a1);
    typedef decltype(a2) VT;
    static_assert(VT::memory_traits::is_unmanaged, "Um");
    REQUIRE(a2.extent_int(0) == 160);
    compare_s_vs_p<16>(a2,a1);
    compare(scalarize(a2),a2);
  }

  {
    const Array2 a1("a2", 10, 4);
    const auto a2 = scalarize(a1);
    typedef decltype(a2) VT;
    static_assert(VT::memory_traits::is_unmanaged, "Um");
    REQUIRE(a2.extent_int(0) == 10);
    REQUIRE(a2.extent_int(1) == 128);
    compare_s_vs_p<32>(a2,a1);
    compare(scalarize(a2),a2);
  }

  {
    const Array3 a1("a3", 3, 2, 4);
    const auto a2 = scalarize(a1);
    typedef decltype(a2) VT;
    static_assert(VT::memory_traits::is_unmanaged, "Um");
    REQUIRE(a2.extent_int(0) == 3);
    REQUIRE(a2.extent_int(1) == 2);
    REQUIRE(a2.extent_int(2) == 32);
    compare_s_vs_p<8>(a2,a1);
    compare(scalarize(a2),a2);
  }

  {
    const Array4 a1("a4", 3, 2, 4, 2);
    const auto a2 = scalarize(a1);
    typedef decltype(a2) VT;
    static_assert(VT::memory_traits::is_unmanaged, "Um");
    REQUIRE(a2.extent_int(0) == 3);
    REQUIRE(a2.extent_int(1) == 2);
    REQUIRE(a2.extent_int(2) == 4);
    REQUIRE(a2.extent_int(3) == 48);
    compare_s_vs_p<24>(a2,a1);
    compare(scalarize(a2),a2);
  }
}

template <int repack_size, typename Src, typename Dst>
OnlyRank<Src, 1> repack_test (const Src& a_src, const Dst& a) {
  static_assert(Dst::memory_traits::is_unmanaged, "Um");
  static_assert(Dst::value_type::n == repack_size, "Pack::n");
  REQUIRE(a.extent_int(0) == (Src::value_type::n/repack_size)*a_src.extent_int(0));
  compare(ekat::scalarize(a_src), ekat::scalarize(a));
  auto sa = ekat::scalarize(a);
  auto rsa = ekat::repack<repack_size>(sa);
  compare_p_vs_p(rsa,a);
}

template <int repack_size, typename Src, typename Dst>
OnlyRank<Src, 2> repack_test (const Src& a_src, const Dst& a) {
  static_assert(Dst::memory_traits::is_unmanaged, "Um");
  static_assert(Dst::value_type::n == repack_size, "Pack::n");
  REQUIRE(a.extent_int(0) == a_src.extent_int(0));
  REQUIRE(a.extent_int(1) == (Src::value_type::n/repack_size)*a_src.extent_int(1));
  compare(ekat::scalarize(a_src), ekat::scalarize(a));
  auto sa = ekat::scalarize(a);
  auto rsa = ekat::repack<repack_size>(sa);
  compare_p_vs_p(rsa,a);
}

TEST_CASE("repack", "ekat::pack") {
  using ekat::Pack;
  using ekat::repack;

  using Array1 = Kokkos::View<Pack<double, 16>*>;
  using Array2 = Kokkos::View<Pack<double, 32>**>;
  using CArray1 = Kokkos::View<Pack<double, 16>*>;
  using CArray2 = Kokkos::View<Pack<double, 32>**>;

  {
    const Array1 a1("a1", 10);
    fill(a1);

    auto run_test = [](const auto v1) {
      const auto v2 = repack<8>(v1);
      repack_test<8>(v1, v2);

      const auto v3 = repack<4>(v2);
      repack_test<4>(v2, v3);

      const auto v4 = repack<2>(v3);
      repack_test<2>(v3, v4);

      const auto v5 = repack<2>(v1);
      repack_test<2>(v1, v5);

      const auto v6 = repack<16>(v1);
      repack_test<16>(v1, v6);
    };

    run_test(a1);
    run_test(CArray1(a1));
  }

  {
    const Array2 a1("v1", 10, 4);
    fill(a1);

    auto run_test = [](const auto v1) {
      const auto v2 = repack<8>(v1);
      repack_test<8>(v1, v2);

      const auto v3 = repack<4>(v2);
      repack_test<4>(v2, v3);

      const auto v4 = repack<2>(v3);
      repack_test<2>(v3, v4);

      const auto v5 = repack<2>(v1);
      repack_test<2>(v1, v5);

      const auto v6 = repack<32>(v1);
      repack_test<32>(v1, v6);
    };

    run_test(a1);
    run_test(CArray2(a1));
  }
}

TEST_CASE("kokkos_packs", "ekat::pack") {
  using namespace ekat;

  using TestBigPack = Pack<Real, 16>;

  using ExeSpace = typename KokkosTypes<DefaultDevice>::ExeSpace;
  using MemberType = typename KokkosTypes<DefaultDevice>::MemberType;

  int nerr = 0;
  const int num_bigs = 17;

  typename KokkosTypes<DefaultDevice>::template view_1d<TestBigPack> test_k_array("test_k_array", num_bigs);
  Kokkos::parallel_reduce("unittest_pack",
                          TeamPolicyFactory<ExeSpace>::get_default_team_policy(1, 1),
                          KOKKOS_LAMBDA(const MemberType& team, int& total_errs) {

    int nerrs_local = 0;

    Kokkos::parallel_for(Kokkos::TeamVectorRange(team, num_bigs), [&] (int i) {
      test_k_array(i) = i;
    });

    auto small = repack<4>(test_k_array);
    if (small.extent_int(0) != 4 * num_bigs) ++nerrs_local;

    team.team_barrier();
    Kokkos::parallel_for(Kokkos::TeamVectorRange(team, num_bigs*4), [&] (int i) {
      for (int p = 0; p < 4; ++p) {
        if (small(i)[p] != i / 4) ++nerrs_local;
      }
    });

    auto big = repack<16>(small);
    if (big.extent_int(0) != num_bigs) ++nerrs_local;

    Kokkos::parallel_for(Kokkos::TeamVectorRange(team, num_bigs*4), [&] (int i) {
      for (int p = 0; p < 4; ++p) {
        small(i)[p] = p * i;
      }
    });

    Kokkos::parallel_for(Kokkos::TeamVectorRange(team, num_bigs*4), [&] (int i) {
      auto mask = small(i) >= (2 * i);
      for (int p = 0; p < 4; ++p) {
        if (i == 0) {
          if (!mask[p]) ++nerrs_local;
        }
        else {
          if (mask[p] != (p >= 2)) ++nerrs_local;
        }
      }
    });

    total_errs += nerrs_local;
  }, nerr);

  // NOTE: catch2 documentation says that its assertion macros are not
  // thread safe, so we have to put them outside of kokkos kernels.
  REQUIRE(nerr == 0);
}

template <typename T>
struct VectorT
{
  using type = T;

  KOKKOS_INLINE_FUNCTION static T get_value(int arg) { return static_cast<T>(arg); }

  KOKKOS_INLINE_FUNCTION static void modify_value(T& value, int arg) { value += static_cast<T>(arg); }
};

template<>
struct VectorT<bool>
{
  using type = char;

  KOKKOS_INLINE_FUNCTION static bool get_value(int arg) { return arg%2 == 0; }

  KOKKOS_INLINE_FUNCTION static void modify_value(bool& value, int arg) {
    bool arg_value = get_value(arg);
    value = (value == arg_value);
  }
};

template <typename T, typename SizeT=int>
void host_device_packs_1d()
{
  using VTS = VectorT<T>;
  using VT = typename VTS::type;

  static constexpr int num_pksizes_to_test = 4;
  static constexpr int num_views_per_pksize = 3;
  static constexpr int fixed_view_size = 67;

  using KT = ekat::KokkosTypes<ekat::DefaultDevice>;

  using Pack1T = ekat::Pack<T, 1>;
  using Pack2T = ekat::Pack<T, 2>;
  using Pack4T = ekat::Pack<T, 4>;
  using Pack8T = ekat::Pack<T, 8>; // we will use this to test fixed-sized view sugar

  using view_p1_t = typename KT::template view_1d<Pack1T>;
  using view_s_t  = typename KT::template view_1d<T>;
  using view_p2_t = typename KT::template view_1d<Pack2T>;
  using view_p4_t = typename KT::template view_1d<Pack4T>;
  using view_p8_t = typename KT::template view_1d<Pack8T>;

  const std::vector<SizeT> sizes        = {13, 37, 59}; // num scalars per view
  const Kokkos::Array<SizeT, 3> sizes_d = {13, 37, 59}; // num scalars per view
  std::vector<std::vector<VT> > raw_data(num_pksizes_to_test, std::vector<VT>());

  // each pksize test (except for the one used to test fixed-size views (Pack8)) has total_flex_scalars
  // of data spread across 3 (num_views_per_pksize) views
  int total_flex_scalars = 0;
  for (int i = 0; i < num_views_per_pksize; ++i) {
    total_flex_scalars += sizes[i];
  }
  static constexpr int total_fixed_scalars = num_views_per_pksize*fixed_view_size;

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    const int mysize = i == num_pksizes_to_test-1 ? total_fixed_scalars : total_flex_scalars;
    raw_data[i].resize(mysize);
  }

  const Kokkos::Array<int, 4> pk_sizes = {1, 2, 4, 8};
  std::vector<view_p1_t> p1_h(num_views_per_pksize);
  std::vector<view_s_t>  p1s_h(num_views_per_pksize);
  std::vector<view_p2_t> p2_h(num_views_per_pksize);
  std::vector<view_p4_t> p4_h(num_views_per_pksize);
  std::vector<view_p8_t> p8_h(num_views_per_pksize); // fixed-size

  std::vector<std::vector<T*> > ptr_data(num_pksizes_to_test, std::vector<T*>(num_views_per_pksize));
  std::vector<std::vector<const T*> > cptr_data(num_pksizes_to_test, std::vector<const T*>(num_views_per_pksize));
  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      if (j == 0) {
        ptr_data[i][j] = reinterpret_cast<T*>(raw_data[i].data());
      }
      else {
        const int last_size = i == num_pksizes_to_test-1 ? fixed_view_size : sizes[j-1];
        ptr_data[i][j] = ptr_data[i][j-1] + last_size;
      }

      cptr_data[i][j] = ptr_data[i][j];
    }
  }

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int klim = (i == num_pksizes_to_test - 1 ? fixed_view_size : sizes[j]);
      for (int k = 0; k < klim; ++k) {
        ptr_data[i][j][k] = VTS::get_value(k*(i+j+1));
      }
    }
  }

  ekat::host_to_device( cptr_data[0], sizes, p1_h);
  ekat::host_to_device( cptr_data[0], sizes, p1s_h);
  ekat::host_to_device( cptr_data[1], sizes, p2_h);
  ekat::host_to_device( cptr_data[2], sizes, p4_h);
  ekat::host_to_device( cptr_data[3], fixed_view_size, p8_h); // fixed-size

  Kokkos::Array<view_p1_t, num_views_per_pksize> p1_d;
  Kokkos::Array<view_s_t,  num_views_per_pksize> p1s_d;
  Kokkos::Array<view_p2_t, num_views_per_pksize> p2_d;
  Kokkos::Array<view_p4_t, num_views_per_pksize> p4_d;
  Kokkos::Array<view_p8_t, num_views_per_pksize> p8_d;

  for (int i = 0; i < num_views_per_pksize; ++i) {
    p1_d[i] = p1_h[i];
    p1s_d[i] = p1s_h[i];
    p2_d[i] = p2_h[i];
    p4_d[i] = p4_h[i];
    p8_d[i] = p8_h[i];
  }

  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int&) {
    for (int i = 0; i < num_pksizes_to_test; ++i) {
      for (int j = 0; j < num_views_per_pksize; ++j) {
        const int sj = sizes_d[j];
        const int klim = (i == num_pksizes_to_test - 1 ? fixed_view_size : sj);
        for (int k = 0; k < klim; ++k) {

          const int view_idx = k / pk_sizes[i];
          const int pk_idx = k % pk_sizes[i];

          if (i == 0) {
            EKAT_KERNEL_REQUIRE(p1_d[j](view_idx)[pk_idx] == VTS::get_value(k*(i+j+1)));
            EKAT_KERNEL_REQUIRE(p1s_d[j](view_idx) == VTS::get_value(k*(i+j+1)));
            p1_d[j](view_idx)[pk_idx] = VTS::get_value(k*(i+j+1) + i + j);
            p1s_d[j](view_idx) = VTS::get_value(k*(i+j+1) + i + j);
          }
          else if (i == 1) {
            EKAT_KERNEL_REQUIRE(p2_d[j](view_idx)[pk_idx] == VTS::get_value(k*(i+j+1)));
            p2_d[j](view_idx)[pk_idx] = VTS::get_value(k*(i+j+1) + i + j);
          }
          else if (i == 2) {
            EKAT_KERNEL_REQUIRE(p4_d[j](view_idx)[pk_idx] == VTS::get_value(k*(i+j+1)));
            p4_d[j](view_idx)[pk_idx] = VTS::get_value(k*(i+j+1) + i + j);
          }
          else if (i == 3) {
            EKAT_KERNEL_REQUIRE(p8_d[j](view_idx)[pk_idx] == VTS::get_value(k*(i+j+1)));
            p8_d[j](view_idx)[pk_idx] = VTS::get_value(k*(i+j+1) + i + j);
          }
          else {
            EKAT_KERNEL_REQUIRE_MSG(false, "Unhandled i");
          }
        }
      }
    }
  });

  ekat::device_to_host( ptr_data[0], sizes, p1_h);
  ekat::device_to_host( ptr_data[1], sizes, p2_h);
  ekat::device_to_host( ptr_data[2], sizes, p4_h);
  ekat::device_to_host( ptr_data[3], fixed_view_size, p8_h); // fixed-size

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int klim = (i == num_pksizes_to_test - 1 ? fixed_view_size : sizes[j]);
      for (int k = 0; k < klim; ++k) {
        REQUIRE(ptr_data[i][j][k] == VTS::get_value(k*(i+j+1) + i + j));
      }
    }
  }

  ekat::device_to_host( ptr_data[0], sizes, p1s_h);
  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int klim = (i == num_pksizes_to_test - 1 ? fixed_view_size : sizes[j]);
      for (int k = 0; k < klim; ++k) {
        REQUIRE(ptr_data[i][j][k] == VTS::get_value(k*(i+j+1) + i + j));
      }
    }
  }
}

TEST_CASE("host_device_packs_1d", "ekat::pack")
{
  host_device_packs_1d<int, size_t>();
  host_device_packs_1d<int, int>();
  host_device_packs_1d<bool>();
}

template <typename T, typename SizeT=int>
void host_device_packs_2d(bool transpose)
{
  using VTS = VectorT<T>;
  using VT = typename VTS::type;

  static constexpr int num_pksizes_to_test = 4;
  static constexpr int num_views_per_pksize = 3;
  static constexpr int fixed_view_dim1 = 5;
  static constexpr int fixed_view_dim2 = 67;

  using KT = ekat::KokkosTypes<ekat::DefaultDevice>;

  using Pack1T = ekat::Pack<T, 1>;
  using Pack2T = ekat::Pack<T, 2>;
  using Pack4T = ekat::Pack<T, 4>;
  using Pack8T = ekat::Pack<T, 8>; // we will use this to test fixed-sized view sugar

  using view_p1_t = typename KT::template view_2d<Pack1T>;
  using view_s_t  = typename KT::template view_2d<T>;
  using view_p2_t = typename KT::template view_2d<Pack2T>;
  using view_p4_t = typename KT::template view_2d<Pack4T>;
  using view_p8_t = typename KT::template view_2d<Pack8T>;

  // dimensions of flex views
  const std::vector<SizeT> dim1_sizes = {3, 4, 5};
  const std::vector<SizeT> dim2_sizes = {13, 37, 59}; // num scalars per view
  const Kokkos::Array<SizeT, 3> dim1_sizes_d = {3, 4, 5};
  const Kokkos::Array<SizeT, 3> dim2_sizes_d = {13, 37, 59}; // num scalars per view

  std::vector<SizeT> total_sizes(num_views_per_pksize);
  for (int i = 0; i < num_views_per_pksize; ++i) {
    total_sizes[i] = dim1_sizes[i] * dim2_sizes[i];
  }

  // place to store raw data
  std::vector<std::vector<VT> > raw_data(num_pksizes_to_test, std::vector<VT>());

  // each pksize test (except for the one used to test fixed-size views (Pack8)) has total_flex_scalars
  // of data spread across 3 (num_views_per_pksize) views
  int total_flex_scalars = 0;
  for (int i = 0; i < num_views_per_pksize; ++i) {
    total_flex_scalars += dim1_sizes[i] * dim2_sizes[i];
  }
  static constexpr int fixed_scalars_per_view = fixed_view_dim1*fixed_view_dim2;
  static constexpr int total_fixed_scalars    = num_views_per_pksize*fixed_scalars_per_view;

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    const int mysize = i == num_pksizes_to_test-1 ? total_fixed_scalars : total_flex_scalars;
    raw_data[i].resize(mysize);
  }

  const Kokkos::Array<int, 4> pk_sizes = {1, 2, 4, 8};
  std::vector<view_p1_t> p1_h(num_views_per_pksize);
  std::vector<view_s_t>  p1s_h(num_views_per_pksize);
  std::vector<view_p2_t> p2_h(num_views_per_pksize);
  std::vector<view_p4_t> p4_h(num_views_per_pksize);
  std::vector<view_p8_t> p8_h(num_views_per_pksize); // fixed-size

  std::vector<std::vector<T*> > ptr_data(num_pksizes_to_test, std::vector<T*>(num_views_per_pksize));
  std::vector<std::vector<const T*> > cptr_data(num_pksizes_to_test, std::vector<const T*>(num_views_per_pksize));
  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      if (j == 0) {
        ptr_data[i][j] = reinterpret_cast<T*>(raw_data[i].data());
      }
      else {
        const int last_size = i == num_pksizes_to_test-1 ? fixed_scalars_per_view : total_sizes[j-1];
        ptr_data[i][j] = ptr_data[i][j-1] + last_size;
      }

      cptr_data[i][j] = ptr_data[i][j];
    }
  }

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes[j]);
      const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes[j]);
      for (int k1 = 0; k1 < kdim1; ++k1) {
        for (int k2 = 0; k2 < kdim2; ++k2) {
          ptr_data[i][j][k1*kdim2 + k2] = VTS::get_value(k1*(i+j+1) + k2*(i-j-1));
        }
      }
    }
  }

  ekat::host_to_device( cptr_data[0], dim1_sizes, dim2_sizes, p1_h, transpose);
  ekat::host_to_device( cptr_data[0], dim1_sizes, dim2_sizes, p1s_h, transpose);
  ekat::host_to_device( cptr_data[1], dim1_sizes, dim2_sizes, p2_h, transpose);
  ekat::host_to_device( cptr_data[2], dim1_sizes, dim2_sizes, p4_h, transpose);
  ekat::host_to_device( cptr_data[3], fixed_view_dim1, fixed_view_dim2, p8_h, transpose); // fixed-size

  Kokkos::Array<view_p1_t, num_views_per_pksize> p1_d;
  Kokkos::Array<view_s_t, num_views_per_pksize> p1s_d;
  Kokkos::Array<view_p2_t, num_views_per_pksize> p2_d;
  Kokkos::Array<view_p4_t, num_views_per_pksize> p4_d;
  Kokkos::Array<view_p8_t, num_views_per_pksize> p8_d;

  for (int i = 0; i < num_views_per_pksize; ++i) {
    p1_d[i] = p1_h[i];
    p1s_d[i] = p1s_h[i];
    p2_d[i] = p2_h[i];
    p4_d[i] = p4_h[i];
    p8_d[i] = p8_h[i];
  }

  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int&) {
    for (int i = 0; i < num_pksizes_to_test; ++i) {
      for (int j = 0; j < num_views_per_pksize; ++j) {
        const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes_d[j]);
        const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes_d[j]);
        for (int k1 = 0; k1 < kdim1; ++k1) {
          for (int k2 = 0; k2 < kdim2; ++k2) {

            const int view_idx = k2 / pk_sizes[i];
            const int pk_idx = k2 % pk_sizes[i];

            T* curr_scalar = nullptr;
            if (i == 0) {
              curr_scalar = &(p1_d[j](k1, view_idx)[pk_idx]);
              T curr = p1s_d[j](k1, view_idx);
              EKAT_KERNEL_REQUIRE(curr == *curr_scalar);
            }
            else if (i == 1) {
              curr_scalar = &(p2_d[j](k1, view_idx)[pk_idx]);
            }
            else if (i == 2) {
              curr_scalar = &(p4_d[j](k1, view_idx)[pk_idx]);
            }
            else if (i == 3) {
              curr_scalar = &(p8_d[j](k1, view_idx)[pk_idx]);
            }
            else {
              EKAT_KERNEL_REQUIRE_MSG(false, "Unhandled i");
            }
            if (!transpose) {
              EKAT_KERNEL_REQUIRE(*curr_scalar == VTS::get_value(k1*(i+j+1) + k2*(i-j-1)));
            }
            VTS::modify_value(*curr_scalar, i + j);
            if (i == 0) {
              p1s_d[j](k1, view_idx) = *curr_scalar;
            }
          }
        }
      }
    }
  });

  ekat::device_to_host( ptr_data[0], dim1_sizes, dim2_sizes, p1_h, transpose);
  ekat::device_to_host( ptr_data[1], dim1_sizes, dim2_sizes, p2_h, transpose);
  ekat::device_to_host( ptr_data[2], dim1_sizes, dim2_sizes, p4_h, transpose);
  ekat::device_to_host( ptr_data[3], fixed_view_dim1, fixed_view_dim2, p8_h, transpose); // fixed-size

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes[j]);
      const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes[j]);
      for (int k1 = 0; k1 < kdim1; ++k1) {
        for (int k2 = 0; k2 < kdim2; ++k2) {
          REQUIRE(ptr_data[i][j][k1*kdim2 + k2] == VTS::get_value(k1*(i+j+1) + k2*(i-j-1) + i + j));
        }
      }
    }
  }

  ekat::device_to_host( ptr_data[0], dim1_sizes, dim2_sizes, p1s_h, transpose);

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes[j]);
      const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes[j]);
      for (int k1 = 0; k1 < kdim1; ++k1) {
        for (int k2 = 0; k2 < kdim2; ++k2) {
          REQUIRE(ptr_data[i][j][k1*kdim2 + k2] == VTS::get_value(k1*(i+j+1) + k2*(i-j-1) + i + j));
        }
      }
    }
  }

}

TEST_CASE("host_device_packs_2d", "ekat::pack")
{
  host_device_packs_2d<bool>(false);
  host_device_packs_2d<bool>(true);
  host_device_packs_2d<int, size_t>(false);
  host_device_packs_2d<int, size_t>(true);
  host_device_packs_2d<int, int>(false);
  host_device_packs_2d<int, int>(true);
}

template <typename T, typename SizeT=int>
void host_device_packs_3d(bool transpose)
{
  using VTS = VectorT<T>;
  using VT = typename VTS::type;

  static constexpr int num_pksizes_to_test = 4;
  static constexpr int num_views_per_pksize = 3;
  static constexpr int fixed_view_dim1 = 3;
  static constexpr int fixed_view_dim2 = 4;
  static constexpr int fixed_view_dim3 = 25;

  using KT = ekat::KokkosTypes<ekat::DefaultDevice>;

  using Pack1T = ekat::Pack<T, 1>;
  using Pack2T = ekat::Pack<T, 2>;
  using Pack4T = ekat::Pack<T, 4>;
  using Pack8T = ekat::Pack<T, 8>; // we will use this to test fixed-sized view sugar

  using view_p1_t = typename KT::template view_3d<Pack1T>;
  using view_s_t  = typename KT::template view_3d<T>;
  using view_p2_t = typename KT::template view_3d<Pack2T>;
  using view_p4_t = typename KT::template view_3d<Pack4T>;
  using view_p8_t = typename KT::template view_3d<Pack8T>;

  // dimensions of flex views
  const std::vector<SizeT> dim1_sizes = {3, 4, 5};
  const std::vector<SizeT> dim2_sizes = {3, 4, 5};
  const std::vector<SizeT> dim3_sizes = {13, 27, 41}; // num scalars per view
  const Kokkos::Array<SizeT, 3> dim1_sizes_d = {3, 4, 5};
  const Kokkos::Array<SizeT, 3> dim2_sizes_d = {3, 4, 5};
  const Kokkos::Array<SizeT, 3> dim3_sizes_d = {13, 27, 41}; // num scalars per view
  std::vector<SizeT> total_sizes(num_views_per_pksize);
  for (int i = 0; i < num_views_per_pksize; ++i) {
    total_sizes[i] = dim1_sizes[i] * dim2_sizes[i] * dim3_sizes[i];
  }

  // place to store raw data
  std::vector<std::vector<VT> > raw_data(num_pksizes_to_test, std::vector<VT>());

  // each pksize test (except for the one used to test fixed-size views (Pack8)) has total_flex_scalars
  // of data spread across 3 (num_views_per_pksize) views
  int total_flex_scalars = 0;
  for (int i = 0; i < num_views_per_pksize; ++i) {
    total_flex_scalars += dim1_sizes[i] * dim2_sizes[i] * dim3_sizes[i];
  }
  static constexpr int fixed_scalars_per_view = fixed_view_dim1*fixed_view_dim2*fixed_view_dim3;
  static constexpr int total_fixed_scalars    = num_views_per_pksize*fixed_scalars_per_view;

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    const int mysize = i == num_pksizes_to_test-1 ? total_fixed_scalars : total_flex_scalars;
    raw_data[i].resize(mysize);
  }

  const Kokkos::Array<int, 4> pk_sizes = {1, 2, 4, 8};
  std::vector<view_p1_t> p1_h(num_views_per_pksize);
  std::vector<view_s_t>  p1s_h(num_views_per_pksize);
  std::vector<view_p2_t> p2_h(num_views_per_pksize);
  std::vector<view_p4_t> p4_h(num_views_per_pksize);
  std::vector<view_p8_t> p8_h(num_views_per_pksize); // fixed-size

  std::vector<std::vector<T*> > ptr_data(num_pksizes_to_test, std::vector<T*>(num_views_per_pksize));
  std::vector<std::vector<const T*> > cptr_data(num_pksizes_to_test, std::vector<const T*>(num_views_per_pksize));
  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      if (j == 0) {
        ptr_data[i][j] = reinterpret_cast<T*>(raw_data[i].data());
      }
      else {
        const int last_size = i == num_pksizes_to_test-1 ? fixed_scalars_per_view : total_sizes[j-1];
        ptr_data[i][j] = ptr_data[i][j-1] + last_size;
      }

      cptr_data[i][j] = ptr_data[i][j];
    }
  }

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes[j]);
      const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes[j]);
      const int kdim3 = (i == num_pksizes_to_test - 1 ? fixed_view_dim3 : dim3_sizes[j]);
      for (int k1 = 0; k1 < kdim1; ++k1) {
        for (int k2 = 0; k2 < kdim2; ++k2) {
          for (int k3 = 0; k3 < kdim3; ++k3) {
            ptr_data[i][j][k1*(kdim2*kdim3) + k2*(kdim3) + k3] = VTS::get_value(k1*(i+j+1) + k2*(i-j-1) + k3*(i+j-1));
          }
        }
      }
    }
  }

  ekat::host_to_device( cptr_data[0], dim1_sizes, dim2_sizes, dim3_sizes, p1_h, transpose);
  ekat::host_to_device( cptr_data[0], dim1_sizes, dim2_sizes, dim3_sizes, p1s_h, transpose);
  ekat::host_to_device( cptr_data[1], dim1_sizes, dim2_sizes, dim3_sizes, p2_h, transpose);
  ekat::host_to_device( cptr_data[2], dim1_sizes, dim2_sizes, dim3_sizes, p4_h, transpose);
  ekat::host_to_device( cptr_data[3], fixed_view_dim1, fixed_view_dim2, fixed_view_dim3, p8_h, transpose); // fixed-size

  Kokkos::Array<view_p1_t, num_views_per_pksize> p1_d;
  Kokkos::Array<view_s_t,  num_views_per_pksize> p1s_d;
  Kokkos::Array<view_p2_t, num_views_per_pksize> p2_d;
  Kokkos::Array<view_p4_t, num_views_per_pksize> p4_d;
  Kokkos::Array<view_p8_t, num_views_per_pksize> p8_d;

  for (int i = 0; i < num_views_per_pksize; ++i) {
    p1_d[i] = p1_h[i];
    p1s_d[i] = p1s_h[i];
    p2_d[i] = p2_h[i];
    p4_d[i] = p4_h[i];
    p8_d[i] = p8_h[i];
  }

  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int&) {
    for (int i = 0; i < num_pksizes_to_test; ++i) {
      for (int j = 0; j < num_views_per_pksize; ++j) {
        const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes_d[j]);
        const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes_d[j]);
        const int kdim3 = (i == num_pksizes_to_test - 1 ? fixed_view_dim3 : dim3_sizes_d[j]);
        for (int k1 = 0; k1 < kdim1; ++k1) {
          for (int k2 = 0; k2 < kdim2; ++k2) {
            for (int k3 = 0; k3 < kdim3; ++k3) {

              const int view_idx = k3 / pk_sizes[i];
              const int pk_idx = k3 % pk_sizes[i];

              T* curr_scalar = nullptr;
              if (i == 0) {
                curr_scalar = &(p1_d[j](k1, k2, view_idx)[pk_idx]);
                T curr = p1s_d[j](k1, k2, view_idx);
                EKAT_KERNEL_REQUIRE(curr == *curr_scalar);
              }
              else if (i == 1) {
                curr_scalar = &(p2_d[j](k1, k2, view_idx)[pk_idx]);
              }
              else if (i == 2) {
                curr_scalar = &(p4_d[j](k1, k2, view_idx)[pk_idx]);
              }
              else if (i == 3) {
                curr_scalar = &(p8_d[j](k1, k2, view_idx)[pk_idx]);
              }
              else {
                EKAT_KERNEL_REQUIRE_MSG(false, "Unhandled i");
              }
              if (!transpose) {
                EKAT_KERNEL_REQUIRE(*curr_scalar == VTS::get_value(k1*(i+j+1) + k2*(i-j-1) + k3*(i+j-1)));
              }
              VTS::modify_value(*curr_scalar, i + j);
              if (i == 0) {
                p1s_d[j](k1, k2, view_idx) = *curr_scalar;
              }
            }
          }
        }
      }
    }
  });

  ekat::device_to_host( ptr_data[0], dim1_sizes, dim2_sizes, dim3_sizes, p1_h, transpose);
  ekat::device_to_host( ptr_data[1], dim1_sizes, dim2_sizes, dim3_sizes, p2_h, transpose);
  ekat::device_to_host( ptr_data[2], dim1_sizes, dim2_sizes, dim3_sizes, p4_h, transpose);
  ekat::device_to_host( ptr_data[3], fixed_view_dim1, fixed_view_dim2, fixed_view_dim3, p8_h, transpose); // fixed-size

  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes[j]);
      const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes[j]);
      const int kdim3 = (i == num_pksizes_to_test - 1 ? fixed_view_dim3 : dim3_sizes[j]);
      for (int k1 = 0; k1 < kdim1; ++k1) {
        for (int k2 = 0; k2 < kdim2; ++k2) {
          for (int k3 = 0; k3 < kdim3; ++k3) {
            REQUIRE(ptr_data[i][j][k1*(kdim2*kdim3) + k2*(kdim3) + k3] == VTS::get_value(k1*(i+j+1) + k2*(i-j-1) + k3*(i+j-1) + i + j));
          }
        }
      }
    }
  }

  ekat::device_to_host( ptr_data[0], dim1_sizes, dim2_sizes, dim3_sizes, p1s_h, transpose);
  for (int i = 0; i < num_pksizes_to_test; ++i) {
    for (int j = 0; j < num_views_per_pksize; ++j) {
      const int kdim1 = (i == num_pksizes_to_test - 1 ? fixed_view_dim1 : dim1_sizes[j]);
      const int kdim2 = (i == num_pksizes_to_test - 1 ? fixed_view_dim2 : dim2_sizes[j]);
      const int kdim3 = (i == num_pksizes_to_test - 1 ? fixed_view_dim3 : dim3_sizes[j]);
      for (int k1 = 0; k1 < kdim1; ++k1) {
        for (int k2 = 0; k2 < kdim2; ++k2) {
          for (int k3 = 0; k3 < kdim3; ++k3) {
            REQUIRE(ptr_data[i][j][k1*(kdim2*kdim3) + k2*(kdim3) + k3] == VTS::get_value(k1*(i+j+1) + k2*(i-j-1) + k3*(i+j-1) + i + j));
          }
        }
      }
    }
  }

}

TEST_CASE("host_device_packs_3d", "ekat::pack")
{
  host_device_packs_3d<bool>(false);
  host_device_packs_3d<bool>(true);
  host_device_packs_3d<int, size_t>(false);
  host_device_packs_3d<int, size_t>(true);
  host_device_packs_3d<int, int>(false);
  host_device_packs_3d<int, int>(true);
}

TEST_CASE("index_and_shift", "ekat::pack")
{
  constexpr int pack_size = 8;
  constexpr int num_ints = 100;
  constexpr int shift = 2;
  using IntPack = ekat::Pack<int, pack_size>;

  Kokkos::View<int*> data("data", num_ints);

  Kokkos::parallel_for(num_ints, KOKKOS_LAMBDA(const int i) {
    data(i) = i + 1000;
  });

  int nerr = 0;
  Kokkos::parallel_reduce(num_ints - shift - pack_size, KOKKOS_LAMBDA(const int i, int& errs) {
    IntPack expected1, expected2, vals1, vals2, idx;
    expected1 = ekat::range<IntPack>(i+1000);
    expected2 = ekat::range<IntPack>(i+1000+shift);
    idx = ekat::range<IntPack>(i);
    ekat::index_and_shift<shift>(data, idx, vals1, vals2);
    if ( (vals1 != expected1 || vals2 != expected2).any()) {
      ++errs;
    }
  }, nerr);
  REQUIRE(nerr == 0);

#ifndef NDEBUG
  // Check index_and_shift sets output to invalid (nan for FP types, finite_max_v for integers) in debug mode
  auto data_h = Kokkos::create_mirror_view(data);
  Kokkos::deep_copy(data_h,data);

  auto data_s = ekat::scalarize(data_h);

  const int num_packs = ekat::npack<IntPack>(num_ints);
  auto invalid = Kokkos::Experimental::finite_max_v<int>;
  for (int ipack=0; ipack<num_packs; ++ipack) {
    auto range = ekat::range<IntPack>(ipack*pack_size);
    IntPack data_i, data_ips;
    ekat::index_and_shift<shift>(data_h,range,data_i,data_ips);
    for (int n=0; n<pack_size; ++n) {
      if ((range[n]+shift)<num_ints) {
        REQUIRE(data_ips[n]==data_s[ipack*pack_size+n+shift]);
      } else {
        REQUIRE (data_ips[n]==invalid);
      }
    }
  }
#endif
}

TEST_CASE("adj_diff", "ekat::pack")
{
  constexpr int N = 8;
  using PT = ekat::Pack<int,N>;

  for (int num_ints : {3*N, 3*N+1}) {

    const auto num_packs = ekat::npack<PT>(num_ints);

    Kokkos::View<int*> data_s("data", num_ints);
    Kokkos::View<PT*>  data_p("data_p",num_packs);

    // Fill inputs with square of integers
    Kokkos::parallel_for(num_ints, KOKKOS_LAMBDA(const int i) {
      data_s(i) = (i+1)*(i+1);
      data_p(i/N)[i%N] = (i+1)*(i+1);
    });
    // Compute adj_differences. Notice that (i+1)^2 - i^2 = 2*i+1
    Kokkos::View<int*> diff_fwd_s("diff_fwd_s", num_ints);
    Kokkos::View<PT*>  diff_fwd_p("diff_fwd_p", num_packs);
    Kokkos::View<int*> diff_bwd_s("diff_bwd_s", num_ints);
    Kokkos::View<PT*>  diff_bwd_p("diff_bwd_p", num_packs);
    auto f_s = KOKKOS_LAMBDA(const int i) {
      diff_fwd_s(i) = ekat::adj_diff<true> (data_s,i);
      diff_bwd_s(i) = ekat::adj_diff<false>(data_s,i);
    };
    auto f_p = KOKKOS_LAMBDA(const int i) {
      diff_fwd_p(i) = ekat::adj_diff<true> (data_p,i);
      diff_bwd_p(i) = ekat::adj_diff<false>(data_p,i);
    };
    Kokkos::parallel_for(num_ints, f_s);
    Kokkos::parallel_for(num_packs, f_p);

    auto diff_fwd_sh = ekat::create_host_mirror_and_copy(diff_fwd_s);
    auto diff_fwd_ph = ekat::create_host_mirror_and_copy(diff_fwd_p);
    auto diff_bwd_sh = ekat::create_host_mirror_and_copy(diff_bwd_s);
    auto diff_bwd_ph = ekat::create_host_mirror_and_copy(diff_bwd_p);
    auto diff_fwd_phs = ekat::scalarize(diff_fwd_ph);
    auto diff_bwd_phs = ekat::scalarize(diff_bwd_ph);

    REQUIRE (diff_fwd_sh(0)==3);
    REQUIRE (diff_fwd_phs(0)==3);
    for (int i=1; i<num_ints-1; ++i) {
      REQUIRE(diff_fwd_sh(i)==2*(i+1)+1);
      REQUIRE(diff_bwd_phs(i)==2*i+1);
      REQUIRE(diff_fwd_phs(i)==2*(i+1)+1);
      REQUIRE(diff_bwd_sh(i)==2*i+1);
    }
    REQUIRE (diff_bwd_sh (num_ints-1)==2*(num_ints-1)+1);
    REQUIRE (diff_bwd_phs(num_ints-1)==2*(num_ints-1)+1);
  }
}

} // namespace
