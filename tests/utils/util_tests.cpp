#include <catch2/catch.hpp>

#include "ekat/ekat_pack.hpp"
#include "ekat/kokkos/ekat_kokkos_meta.hpp"
#include "ekat/ekat_parameter_list.hpp"
#include "ekat/ekat_type_traits.hpp"

#include "ekat_test_config.h"

namespace {

TEST_CASE("precision", "util") {
  CHECK_FALSE(ekat::is_single_precision<double>::value);
  CHECK(ekat::is_single_precision<float>::value);
}

TEST_CASE("type_traits", "") {
  using namespace ekat;
  REQUIRE(std::is_same<ekat::ValueType<double**&>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double*[3]>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double[2][3]>::type,double>::value);

  // Check meta-util to get rank and dynamic rank of a raw MD array
  REQUIRE(ekat::GetRanks<double[2][3]>::rank==2);
  REQUIRE(ekat::GetRanks<double[2][3]>::rank_dynamic==0);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank==3);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank_dynamic==1);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank==4);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank_dynamic==2);
}

// This is just a compilation test.
TEST_CASE("Unmanaged", "ekat::ko") {
  using ekat::Unmanaged;

  {
    typedef Kokkos::View<double*> V;
    V v("v", 10);
    typedef Unmanaged<V> VUm;
    VUm v_um(v);
    static_assert( ! V::traits::memory_traits::is_unmanaged, "Um");
    static_assert(VUm::traits::memory_traits::is_unmanaged, "Um");
  }

  {
    typedef Kokkos::View<ekat::Pack<double, EKAT_TEST_PACK_SIZE>***,
                         Kokkos::LayoutLeft,
                         Kokkos::HostSpace,
                         Kokkos::MemoryTraits<Kokkos::RandomAccess> >
      V;
    V v("v", 2, 3, 4);
    typedef Unmanaged<V> VUm;
    static_assert(VUm::traits::memory_traits::is_random_access, "Um");
    static_assert(VUm::traits::memory_traits::is_unmanaged, "Um");
    VUm v_um(v);
    typedef Unmanaged<VUm> VUmUm;
    static_assert(VUmUm::traits::memory_traits::is_random_access, "Um");
    static_assert(VUmUm::traits::memory_traits::is_unmanaged, "Um");
    static_assert( ! VUmUm::traits::memory_traits::is_atomic, "Um");
    static_assert( ! VUmUm::traits::memory_traits::is_aligned, "Um");
    static_assert( ! VUmUm::traits::memory_traits::is_restrict, "Um");
    VUmUm v_umum(v);
  }

  {
    typedef Kokkos::View<ekat::Pack<int, EKAT_TEST_PACK_SIZE>[10],
                         Kokkos::HostSpace,
                         Kokkos::MemoryTraits<Kokkos::Atomic | Kokkos::Aligned | Kokkos::Restrict> >
      V;
    static_assert( ! V::traits::memory_traits::is_unmanaged, "Um");
    V v("v");
    typedef Unmanaged<V>::const_type CVUm;
    static_assert(CVUm::traits::memory_traits::is_atomic, "Um");
    static_assert(CVUm::traits::memory_traits::is_aligned, "Um");
    static_assert(CVUm::traits::memory_traits::is_restrict, "Um");
    static_assert(CVUm::traits::memory_traits::is_unmanaged, "Um");

    using Kokkos::Impl::ViewMapping;
    static_assert(ViewMapping<CVUm::traits, V::traits, void>::is_assignable,
                  "CVUm <- V");
    static_assert( ! ViewMapping<V::traits, CVUm::traits, void>::is_assignable,
                  "V </- CVUm");
    static_assert(ViewMapping<CVUm::traits, Unmanaged<V>::traits, void>::is_assignable,
                  "CVUm <- VUm");
    static_assert( ! ViewMapping<Unmanaged<V>::traits, CVUm::traits, void>::is_assignable,
                  "VUm </- CVUm");
    CVUm cv_um(v);
  }
}

TEST_CASE("parameter_list", "") {
  using namespace ekat;

  ParameterList src("src");
  src.set<int>("i",8);
  src.set<int>("j",10);
  src.sublist("sl").set<double>("d",1.0);

  ParameterList dst("dst");
  dst.set<int>("i",10);

  dst.import(src);

  REQUIRE (dst.get<int>("i")==8);

  REQUIRE (dst.isParameter("j"));
  REQUIRE (dst.get<int>("j")==10);

  REQUIRE (dst.isSublist("sl"));
  REQUIRE (dst.sublist("sl").isParameter("d"));
  REQUIRE (dst.sublist("sl").get<double>("d")==1.0);

  auto sl_begin = src.sublists_names_cbegin();
  auto sl_end   = src.sublists_names_cend();
  REQUIRE (std::next(sl_begin,1)==sl_end); // Only one sublist

  auto p_begin = src.params_names_cbegin();
  auto p_end   = src.params_names_cend();
  REQUIRE (std::next(p_begin,2)==p_end); // Two params
}

} // empty namespace
