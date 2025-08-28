#include <catch2/catch.hpp>

#include "ekat_view_broadcast.hpp"

namespace {

TEST_CASE("view_broadcast") {
  using namespace ekat;

  SECTION ("from_1d") {
    using from_t = Kokkos::View<int*>;
    const int n = 5;
    from_t orig("",n);
    auto orig_h = Kokkos::create_mirror_view(orig);
    for (int i=0; i<n; ++i) { orig_h(i) = i; }
    Kokkos::deep_copy(orig,orig_h);

    SECTION ("to_1d") {
      using view_1d = Kokkos::View<int*>;
      using b1d_t = ViewBroadcast<view_1d>;

      REQUIRE_THROWS(b1d_t(orig,{0,1}));  // Badly sized extents
      REQUIRE_THROWS(b1d_t(orig,{1}));    // Too many positive extents

      int d0 = n;
      b1d_t b1d(orig,{-1});

      int err = 0;
      auto lambda = KOKKOS_LAMBDA (int i, int& l_err) {
        if (b1d(i)!=orig(i)) ++l_err;
      };
      Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0),lambda,err);
      REQUIRE (err==0);
    }

    SECTION ("to_2d") {
      using view_2d = Kokkos::View<int**>;
      using b2d_t = ViewBroadcast<view_2d>;

      REQUIRE_THROWS(b2d_t(orig,{0,1,2})); // Badly sized extents
      REQUIRE_THROWS(b2d_t(orig,{1,2}));   // Too many positive extents
      REQUIRE_THROWS(b2d_t(orig,{0,0}));   // Too few positive extents

      int d0,d1;
     
      SECTION ("along d0") {
        d0 = 4;
        d1 = n;
        b2d_t b2d(orig,{d0,-1});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = idx / d1;
          int j = idx % d1;
          if (b2d(i,j)!=orig(j)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1),lambda,err);
        REQUIRE (err==0);
      }
      SECTION ("along d1") {
        d0 = n;
        d1 = 4;
        b2d_t b2d(orig,{-1,d1});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = idx / d1;
          int j = idx % d1;
          if (b2d(i,j)!=orig(i)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1),lambda,err);
        REQUIRE (err==0);
      }
    }
    SECTION("to_3d") {
      using view_3d = Kokkos::View<int***>;
      using b3d_t = ViewBroadcast<view_3d>;

      REQUIRE_THROWS(b3d_t(orig,{0,1,2,3})); // Badly sized extents
      REQUIRE_THROWS(b3d_t(orig,{1,2,3}));   // Too many positive extents
      REQUIRE_THROWS(b3d_t(orig,{0,0,1}));   // Too few positive extents

      int d0,d1,d2;
     
      SECTION ("along d0-d1") {
        d0 = 4;
        d1 = 3;
        d2 = n;
        b3d_t b3d(orig,{d0,d1,-1});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = (idx / d2) / d1;
          int j = (idx / d2) % d1;
          int k = idx % d2;
          if (b3d(i,j,k)!=orig(k)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1*d2),lambda,err);
        REQUIRE (err==0);
      }
      SECTION ("along d0-d2") {
        d0 = 4;
        d1 = n;
        d2 = 3;
        b3d_t b3d(orig,{d0,-1,d1});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = (idx / d2) / d1;
          int j = (idx / d2) % d1;
          int k = idx % d2;
          if (b3d(i,j,k)!=orig(j)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1*d2),lambda,err);
        REQUIRE (err==0);
      }
      SECTION ("along d1-d2") {
        d0 = n;
        d1 = 4;
        d2 = 3;
        b3d_t b3d(orig,{-1,d1,d2});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = (idx / d2) / d1;
          int j = (idx / d2) % d1;
          int k = idx % d2;
          if (b3d(i,j,k)!=orig(i)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1*d2),lambda,err);
        REQUIRE (err==0);
      }
    }
  }

  SECTION ("from_2d") {
    using from_t = Kokkos::View<int**>;

    const int m = 4;
    const int n = 5;
    from_t orig("",m,n);
    auto orig_h = Kokkos::create_mirror_view(orig);
    for (int i=0; i<m*n; ++i) { orig_h.data()[i] = i; }
    Kokkos::deep_copy(orig,orig_h);

    SECTION ("to_2d") {
      using view_2d = Kokkos::View<int**>;
      using b2d_t = ViewBroadcast<view_2d>;

      REQUIRE_THROWS(b2d_t(orig,{0,1,2})); // Badly sized extents
      REQUIRE_THROWS(b2d_t(orig,{1,0}));   // Too many positive extents

      int d0 = m;
      int d1 = n;
      b2d_t b1d(orig,{-1,-1});

      int err = 0;
      auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
        int i = idx / d1;
        int j = idx % d1;
        if (b1d(i,j)!=orig(i,j)) ++l_err;
      };
      Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1),lambda,err);
      REQUIRE (err==0);
    }

    SECTION ("to_3d") {
      using view_3d = Kokkos::View<int***>;
      using b3d_t = ViewBroadcast<view_3d>;

      REQUIRE_THROWS(b3d_t(orig,{0,1,2,3})); // Badly sized extents
      REQUIRE_THROWS(b3d_t(orig,{1,2,0}));   // Too many positive extents
      REQUIRE_THROWS(b3d_t(orig,{0,0,0}));   // Too few positive extents

      int d0,d1,d2;
     
      SECTION ("along d0") {
        d0 = 3;
        d1 = m;
        d2 = n;
        b3d_t b3d(orig,{d0,-1,-1});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = (idx / d2) / d1;
          int j = (idx / d2) % d1;
          int k = idx % d2;
          if (b3d(i,j,k)!=orig(j,k)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1*d2),lambda,err);
        REQUIRE (err==0);
      }
      SECTION ("along d1") {
        d0 = m;
        d1 = 3;
        d2 = n;
        b3d_t b3d(orig,{-1,d1,-1});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = (idx / d2) / d1;
          int j = (idx / d2) % d1;
          int k = idx % d2;
          if (b3d(i,j,k)!=orig(i,k)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1*d2),lambda,err);
        REQUIRE (err==0);
      }
      SECTION ("along d2") {
        d0 = m;
        d1 = n;
        d2 = 3;
        b3d_t b3d(orig,{-1,-1,d2});

        int err = 0;
        auto lambda = KOKKOS_LAMBDA (int idx, int& l_err) {
          int i = (idx / d2) / d1;
          int j = (idx / d2) % d1;
          int k = idx % d2;
          if (b3d(i,j,k)!=orig(i,j)) ++l_err;
        };
        Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0,d0*d1*d2),lambda,err);
        REQUIRE (err==0);
      }
    }
  }
}

} // anonymous namespace
