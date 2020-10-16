#include <catch2/catch.hpp>

#include "ekat/util/ekat_math_utils.hpp"
#include "ekat/kokkos/ekat_kokkos_types.hpp"

#include "ekat_test_config.h"

#include <vector>

using namespace ekat;

namespace {

TEST_CASE("transpose_2d", "math_util")
{
  using KT       = KokkosTypes<HostDevice>;
  using CViewT   = Unmanaged<typename KT::view<Int**> >;
  using F90ViewT = Unmanaged<typename KT::lview<Int**> >;

  const Int rows = 17;
  const Int cols = 27;
  const Int total = rows*cols;

  std::vector<Int> dataC(total), dataF90(total), dataC2(total);
  for (Int i = 0; i < total; ++i) {
    dataC[i] = i;
  }

  transpose<TransposeDirection::c2f>(dataC.data(), dataF90.data(), rows, cols);
  CViewT cview(dataC.data(), rows, cols);
  F90ViewT f90view(dataF90.data(), rows, cols);

  for (Int i = 0; i < rows; ++i) {
    for (Int j = 0; j < cols; ++j) {
      REQUIRE(cview(i, j) == f90view(i, j));
    }
  }

  transpose<TransposeDirection::f2c>(dataF90.data(), dataC2.data(), rows, cols);
  CViewT cview2(dataC2.data(), rows, cols);

  for (Int i = 0; i < rows; ++i) {
    for (Int j = 0; j < cols; ++j) {
      REQUIRE(cview(i, j) == cview2(i, j));
    }
  }
}

TEST_CASE("transpose_3d", "math_util")
{
  using KT       = KokkosTypes<HostDevice>;
  using CViewT   = Unmanaged<typename KT::view<Int***> >;
  using F90ViewT = Unmanaged<typename KT::lview<Int***> >;

  const Int rows   = 17;
  const Int cols   = 27;
  const Int slices = 7;
  const Int total  = rows*cols*slices;

  std::vector<Int> dataC(total), dataF90(total), dataC2(total);
  for (Int i = 0; i < total; ++i) {
    dataC[i] = i;
  }

  transpose<TransposeDirection::c2f>(dataC.data(), dataF90.data(), rows, cols, slices);
  CViewT cview(dataC.data(), rows, cols, slices);
  F90ViewT f90view(dataF90.data(), rows, cols, slices);

  for (Int i = 0; i < rows; ++i) {
    for (Int j = 0; j < cols; ++j) {
      for (Int k = 0; k < slices; ++k) {
        REQUIRE(cview(i, j, k) == f90view(i, j, k));
      }
    }
  }

  transpose<TransposeDirection::f2c>(dataF90.data(), dataC2.data(), rows, cols, slices);
  CViewT cview2(dataC2.data(), rows, cols, slices);

  for (Int i = 0; i < rows; ++i) {
    for (Int j = 0; j < cols; ++j) {
      for (Int k = 0; k < slices; ++k) {
        REQUIRE(cview(i, j, k) == cview2(i, j, k));
      }
    }
  }
}

} // empty namespace
