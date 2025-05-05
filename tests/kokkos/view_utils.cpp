#include <catch2/catch.hpp>

#include "ekat_view_utils.hpp"

#include "ekat_test_config.h"

namespace {

TEST_CASE("cmvdc", "[kokkos_utils]") {
  using namespace ekat;

  // Check utility that creates a host mirror and deep copies
  Kokkos::View<double*> v("",100);
  auto fill = KOKKOS_LAMBDA(int i) { v(i) = i; };
  Kokkos::parallel_for(Kokkos::RangePolicy<>(0,100),fill);

  auto vh = cmvdc(v);
  for (int i=0; i<100; ++i) {
    REQUIRE (vh(i)==i);
  }
}

TEST_CASE("reshape", "[kokkos_utils]") {
  using namespace ekat;

  // Check util that allows to reshape a view
  Kokkos::View<double*> v1d("",100);
  auto v2d = reshape<double[2][50]>(v1d);
  REQUIRE(v2d.size()==100);

  auto v3d = reshape<double*[5][5]>(v2d,4);
  REQUIRE (v3d.size()==100);
}

} // anonymous namespace
