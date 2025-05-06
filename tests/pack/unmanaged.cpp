#include <catch2/catch.hpp>

#include "ekat_pack.hpp"
#include "ekat_kokkos_meta.hpp"

#include "ekat_test_config.h"

namespace {

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

} // empty namespace
