#include <catch2/catch.hpp>

#include "ekat_subview_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include "ekat_test_config.h"

namespace {

TEST_CASE("subviews") {
  using kt = ekat::KokkosTypes<ekat::DefaultDevice>;

  const int i0 = 5;
  const int i1 = 4;
  const int i2 = 3;
  const int i3 = 2;
  const int i4 = 1;
  const int i5 = 0;

  // Create input view
  kt::view_ND<double,6> v6("v6",7,6,5,4,3,2);
  const int s = v6.size();
  Kokkos::parallel_for (kt::RangePolicy(0,s),
                        KOKKOS_LAMBDA(int i) {
    *(v6.data()+i) = i;
  });

  auto v5 = ekat::subview(v6,i0);
  auto v4 = ekat::subview(v6,i0,i1);
  auto v3 = ekat::subview(v6,i0,i1,i2);
  auto v2 = ekat::subview(v6,i0,i1,i2,i3);
  auto v1 = ekat::subview(v6,i0,i1,i2,i3,i4);

  SECTION ("subview_major") {

    // Subviews of v5
    auto v5_4 = ekat::subview(v5,i1);
    auto v5_3 = ekat::subview(v5,i1,i2);
    auto v5_2 = ekat::subview(v5,i1,i2,i3);
    auto v5_1 = ekat::subview(v5,i1,i2,i3,i4);

    // Subviews of v4
    auto v4_3 = ekat::subview(v4,i2);
    auto v4_2 = ekat::subview(v4,i2,i3);
    auto v4_1 = ekat::subview(v4,i2,i3,i4);

    // Subviews of v3
    auto v3_2 = ekat::subview(v3,i3);
    auto v3_1 = ekat::subview(v3,i3,i4);

    // Subviews of v2
    auto v2_1 = ekat::subview(v2,i4);

    // Compare with original view
    Kokkos::View<int> diffs("");
    Kokkos::deep_copy(diffs,0);
    Kokkos::parallel_for(kt::RangePolicy(0,1),
                         KOKKOS_LAMBDA(int) {

      int& ndiffs = diffs();
      // Check vN and vN_k against v6
      for (int m=0; m<2; ++m) {
        for (int l=0; l<3; ++l) {
          for (int k=0; k<4; ++k) {
            for (int j=0; j<5; ++j) {
              for (int i=0; i<6; ++i) {
                if (v5(i,j,k,l,m)!=v6(i0,i,j,k,l,m)) ++ndiffs;
              }
              if (v4(j,k,l,m)!=v6(i0,i1,j,k,l,m)) ++ndiffs;
              if (v5_4(j,k,l,m)!=v6(i0,i1,j,k,l,m)) ++ndiffs;
            }
            if (v3(k,l,m)!=v6(i0,i1,i2,k,l,m)) ++ndiffs;
            if (v5_3(k,l,m)!=v6(i0,i1,i2,k,l,m)) ++ndiffs;
            if (v4_3(k,l,m)!=v6(i0,i1,i2,k,l,m)) ++ndiffs;
          }
          if (v2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
          if (v5_2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
          if (v4_2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
          if (v3_2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
        }
        if (v1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v5_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v4_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v3_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v2_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;

        // Check rank0 subview
        auto v0 = ekat::subview(v1,m);
        if (v0()!=v1(m)) ++ndiffs;
      }

      // Make sure that our diffs counting strategy works
      // by checking that two entries that should be different
      // are indeed different.
      if (v2_1(0) != v6(i0,i1,i2,i3,i4,1)) ++ndiffs;
    });
    auto diffs_h = Kokkos::create_mirror_view(diffs);
    Kokkos::deep_copy(diffs_h,diffs);
    REQUIRE (diffs_h()==1);
  }

  SECTION ("second_slowest") {
    // Subview the second slowest
    auto sv6 = ekat::subview_1(v6,i1);
    auto sv5 = ekat::subview_1(v5,i2);
    auto sv4 = ekat::subview_1(v4,i3);
    auto sv3 = ekat::subview_1(v3,i4);
    auto sv2 = ekat::subview_1(v2,i5);

    // First four should retain LaoutRight, last one has no other choice but getting LayoutStride
    REQUIRE (std::is_same<typename decltype(sv6)::traits::array_layout,Kokkos::LayoutRight>::value);
    REQUIRE (std::is_same<typename decltype(sv5)::traits::array_layout,Kokkos::LayoutRight>::value);
    REQUIRE (std::is_same<typename decltype(sv4)::traits::array_layout,Kokkos::LayoutRight>::value);
    REQUIRE (std::is_same<typename decltype(sv3)::traits::array_layout,Kokkos::LayoutRight>::value);
    REQUIRE (std::is_same<typename decltype(sv2)::traits::array_layout,Kokkos::LayoutStride>::value);

    // Subview again the second slowest
    auto sv6_2 = ekat::subview_1(sv6,i2);
    auto sv5_2 = ekat::subview_1(sv5,i3);
    auto sv4_2 = ekat::subview_1(sv4,i4);

    // Compare with original view
    Kokkos::View<int> diffs("");
    Kokkos::deep_copy(diffs,0);
    Kokkos::parallel_for(kt::RangePolicy(0,1),
                         KOKKOS_LAMBDA(int) {

      int& ndiffs = diffs();
      for (int h=0; h<7; ++h)
        for (int j=0; j<5; ++j)
          for (int k=0; k<4; ++k)
            for (int l=0; l<3; ++l)
              for (int m=0; m<2; ++m) {
                if (sv6(h,j,k,l,m)!=v6(h,i1,j,k,l,m)) ++ndiffs;
              }
      for (int i=0; i<5; ++i)
        for (int k=0; k<4; ++k)
          for (int l=0; l<3; ++l)
            for (int m=0; m<2; ++m) {
              if (sv5(i,k,l,m)!=v6(i0,i,i2,k,l,m)) ++ndiffs;
            }
      for (int j=0; j<5; ++j)
        for (int l=0; l<3; ++l)
          for (int m=0; m<2; ++m) {
            if (sv4(j,l,m)!=v6(i0,i1,j,i3,l,m)) ++ndiffs;
          }
      for (int k=0; k<4; ++k)
        for (int m=0; m<2; ++m) {
          if (sv3(k,m)!=v6(i0,i1,i2,k,i4,m)) ++ndiffs;
        }

      for (int l=0; l<3; ++l) {
        if (sv2(l)!=v6(i0,i1,i2,i3,l,i5)) ++ndiffs;
      }

      for (int h=0; h<7; ++h)
        for (int k=0; k<4; ++k)
          for (int l=0; l<3; ++l)
            for (int m=0; m<2; ++m) {
              if (sv6_2(h,k,l,m)!=v6(h,i1,i2,k,l,m)) ++ndiffs;
            }
      for (int i=0; i<4; ++i)
        for (int l=0; l<3; ++l)
          for (int m=0; m<2; ++m) {
            if (sv5_2(i,l,m)!=v6(i0,i,i2,i3,l,m)) ++ndiffs;
          }
      for (int j=0; j<5; ++j)
        for (int m=0; m<2; ++m) {
          if (sv4_2(j,m)!=v6(i0,i1,j,i3,i4,m)) ++ndiffs;
        }

      // Make sure that our diffs counting strategy works
      // by checking that two entries that should be different
      // are indeed different.
      if (sv4_2(0,0)!=v6(i0,i1,0,i3,i4,1)) ++ndiffs;
    });
    auto diffs_h = Kokkos::create_mirror_view(diffs);
    Kokkos::deep_copy(diffs_h,diffs);
    REQUIRE (diffs_h()==1);
  }
}

TEST_CASE("multi-slice-subviews") {
  using kt = ekat::KokkosTypes<ekat::DefaultDevice>;

  const int i0 = 5;
  const int i1 = 4;
  const int i2 = 3;
  const int i3 = 2;
  const int i4 = 1;
  const int idx0[6] = {0, 4, 1, 0, 1, 0};
  const int idx1[6] = {3, 6, 4, 2, 2, 1};

  auto p0 = Kokkos::make_pair<int, int>(idx0[0], idx1[0]);
  auto p1 = Kokkos::make_pair<int, int>(idx0[1], idx1[1]);
  auto p2 = Kokkos::make_pair<int, int>(idx0[2], idx1[2]);
  auto p3 = Kokkos::make_pair<int, int>(idx0[3], idx1[3]);
  auto p4 = Kokkos::make_pair<int, int>(idx0[4], idx1[4]);
  auto p5 = Kokkos::make_pair<int, int>(idx0[5], idx1[5]);

  // Create input view
  kt::view_ND<double, 6> v6("v6", 7, 6, 5, 4, 3, 2);
  const int s = v6.size();
  Kokkos::parallel_for(
      kt::RangePolicy(0, s), KOKKOS_LAMBDA(int i) { *(v6.data() + i) = i; });

  auto v5 = ekat::subview(v6, i0);
  auto v4 = ekat::subview(v6, i0, i1);
  auto v3 = ekat::subview(v6, i0, i1, i2);
  auto v2 = ekat::subview(v6, i0, i1, i2, i3);
  auto v1 = ekat::subview(v6, i0, i1, i2, i3, i4);

  SECTION("subview_major") {
    // Subviews of v6
    auto v6_5 = ekat::subview(v6, p5, 5);
    auto v6_4 = ekat::subview(v6, p4, 4);
    auto v6_3 = ekat::subview(v6, p3, 3);
    auto v6_2 = ekat::subview(v6, p2, 2);
    auto v6_1 = ekat::subview(v6, p1, 1);
    auto v6_0 = ekat::subview(v6, p0, 0);

    // Subviews of v5
    auto v5_4 = ekat::subview(v5, p5, 4);
    auto v5_3 = ekat::subview(v5, p4, 3);
    auto v5_2 = ekat::subview(v5, p3, 2);
    auto v5_1 = ekat::subview(v5, p2, 1);
    auto v5_0 = ekat::subview(v5, p1, 0);

    // Subviews of v4
    auto v4_3 = ekat::subview(v4, p5, 3);
    auto v4_2 = ekat::subview(v4, p4, 2);
    auto v4_1 = ekat::subview(v4, p3, 1);
    auto v4_0 = ekat::subview(v4, p2, 0);

    // Subviews of v3
    auto v3_2 = ekat::subview(v3, p5, 2);
    auto v3_1 = ekat::subview(v3, p4, 1);
    auto v3_0 = ekat::subview(v3, p3, 0);

    // Subviews of v2
    auto v2_1 = ekat::subview(v2, p5, 1);
    auto v2_0 = ekat::subview(v2, p4, 0);

    // Subviews of v1
    auto v1_0 = ekat::subview(v1, p5);

    // Compare with original views and count diffs
    Kokkos::View<int> diffs("");
    Kokkos::deep_copy(diffs, 0);
    Kokkos::parallel_for(
        kt::RangePolicy(0, 1), KOKKOS_LAMBDA(int) {
          int i1_, i2_, j1, j2, k1, k2, l1, l2, m1, m2, n1, n2;

          auto testv6(v6_0);
          auto testv5(v5_0);
          auto testv4(v4_0);
          auto testv3(v3_0);
          auto testv2(v2_0);
          auto testv1(v1_0);

          int& ndiffs = diffs();
          for (int ens = 0; ens < 6; ens++) {
            i1_ = (ens == 0) ? idx0[0] : 0;
            i2_ = (ens == 0) ? idx1[0] : 7;
            if (ens == 0)
              testv6 = v6_0;
            j1 = (ens == 1) ? idx0[1] : 0;
            j2 = (ens == 1) ? idx1[1] : 6;
            if (ens == 1)
              testv6 = v6_1;
            k1 = (ens == 2) ? idx0[2] : 0;
            k2 = (ens == 2) ? idx1[2] : 5;
            if (ens == 2)
              testv6 = v6_2;
            l1 = (ens == 3) ? idx0[3] : 0;
            l2 = (ens == 3) ? idx1[3] : 4;
            if (ens == 3)
              testv6 = v6_3;
            m1 = (ens == 4) ? idx0[4] : 0;
            m2 = (ens == 4) ? idx1[4] : 3;
            if (ens == 4)
              testv6 = v6_4;
            n1 = (ens == 5) ? idx0[5] : 0;
            n2 = (ens == 5) ? idx1[5] : 2;
            if (ens == 5)
              testv6 = v6_5;
            for (int n = n1; n < n2; n++)
              for (int m = m1; m < m2; m++)
                for (int l = l1; l < l2; l++)
                  for (int k = k1; k < k2; k++)
                    for (int j = j1; j < j2; j++)
                      for (int i = i1_; i < i2_; i++) {
                        if (v6(i, j, k, l, m, n) != testv6(i - i1_, j - j1,
                                                           k - k1, l - l1,
                                                           m - m1, n - n1))
                          ++ndiffs;
                      }
          }
          for (int ens = 1; ens < 6; ens++) {
            j1 = (ens == 1) ? idx0[1] : 0;
            j2 = (ens == 1) ? idx1[1] : 6;
            if (ens == 1)
              testv5 = v5_0;
            k1 = (ens == 2) ? idx0[2] : 0;
            k2 = (ens == 2) ? idx1[2] : 5;
            if (ens == 2)
              testv5 = v5_1;
            l1 = (ens == 3) ? idx0[3] : 0;
            l2 = (ens == 3) ? idx1[3] : 4;
            if (ens == 3)
              testv5 = v5_2;
            m1 = (ens == 4) ? idx0[4] : 0;
            m2 = (ens == 4) ? idx1[4] : 3;
            if (ens == 4)
              testv5 = v5_3;
            n1 = (ens == 5) ? idx0[5] : 0;
            n2 = (ens == 5) ? idx1[5] : 2;
            if (ens == 5)
              testv5 = v5_4;
            for (int j = j1; j < j2; j++)
              for (int k = k1; k < k2; k++)
                for (int l = l1; l < l2; l++)
                  for (int m = m1; m < m2; m++)
                    for (int n = n1; n < n2; n++) {
                      if (v5(j, k, l, m, n) !=
                          testv5(j - j1, k - k1, l - l1, m - m1, n - n1))
                        ++ndiffs;
                    }
          }
          for (int ens = 2; ens < 6; ens++) {
            k1 = (ens == 2) ? idx0[2] : 0;
            k2 = (ens == 2) ? idx1[2] : 5;
            if (ens == 2)
              testv4 = v4_0;
            l1 = (ens == 3) ? idx0[3] : 0;
            l2 = (ens == 3) ? idx1[3] : 4;
            if (ens == 3)
              testv4 = v4_1;
            m1 = (ens == 4) ? idx0[4] : 0;
            m2 = (ens == 4) ? idx1[4] : 3;
            if (ens == 4)
              testv4 = v4_2;
            n1 = (ens == 5) ? idx0[5] : 0;
            n2 = (ens == 5) ? idx1[5] : 2;
            if (ens == 5)
              testv4 = v4_3;
            for (int k = k1; k < k2; k++)
              for (int l = l1; l < l2; l++)
                for (int m = m1; m < m2; m++)
                  for (int n = n1; n < n2; n++) {
                    if (v4(k, l, m, n) !=
                        testv4(k - k1, l - l1, m - m1, n - n1))
                      ++ndiffs;
                  }
          }
          for (int ens = 3; ens < 6; ens++) {
            l1 = (ens == 3) ? idx0[3] : 0;
            l2 = (ens == 3) ? idx1[3] : 4;
            if (ens == 3)
              testv3 = v3_0;
            m1 = (ens == 4) ? idx0[4] : 0;
            m2 = (ens == 4) ? idx1[4] : 3;
            if (ens == 4)
              testv3 = v3_1;
            n1 = (ens == 5) ? idx0[5] : 0;
            n2 = (ens == 5) ? idx1[5] : 2;
            if (ens == 5)
              testv3 = v3_2;
            for (int l = l1; l < l2; l++)
              for (int m = m1; m < m2; m++)
                for (int n = n1; n < n2; n++) {
                  if (v3(l, m, n) != testv3(l - l1, m - m1, n - n1))
                    ++ndiffs;
                }
          }
          for (int ens = 4; ens < 6; ens++) {
            m1 = (ens == 4) ? idx0[4] : 0;
            m2 = (ens == 4) ? idx1[4] : 3;
            if (ens == 4)
              testv2 = v2_0;
            n1 = (ens == 5) ? idx0[5] : 0;
            n2 = (ens == 5) ? idx1[5] : 2;
            if (ens == 5)
              testv2 = v2_1;
            for (int m = m1; m < m2; m++)
              for (int n = n1; n < n2; n++) {
                if (v2(m, n) != testv2(m - m1, n - n1))
                  ++ndiffs;
              }
          }
          n1 = idx0[5];
          n2 = idx1[5];
          testv1 = v1_0;
          for (int n = n1; n < n2; n++) {
            if (v1(n) != testv1(n - n1))
              ++ndiffs;
          }
          // Make sure that our diffs counting strategy works
          // by checking that two entries that should be different
          // are indeed different.
          if (v1_0(0) != v6(i0, i1, i2, i3, i4, 1))
            ++ndiffs;
        });
    auto diffs_h = Kokkos::create_mirror_view(diffs);
    Kokkos::deep_copy(diffs_h, diffs);
    REQUIRE(diffs_h() == 1);
  }
}

} // anonymous namespace
