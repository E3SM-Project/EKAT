#include <catch2/catch.hpp>

#include "ekat_math_utils.hpp"
#include "ekat_kokkos_meta.hpp"
#include "ekat_kokkos_types.hpp"

#include "ekat_test_config.h"

#include <vector>

using namespace ekat;

namespace {

TEST_CASE("transpose_2d", "math_util")
{
  using KT       = KokkosTypes<HostDevice>;
  using CViewT   = Unmanaged<typename KT::view<int**> >;
  using F90ViewT = Unmanaged<typename KT::lview<int**> >;

  const int rows = 17;
  const int cols = 27;
  const int total = rows*cols;

  std::vector<int> dataC(total), dataF90(total), dataC2(total);
  for (int i = 0; i < total; ++i) {
    dataC[i] = i;
  }

  transpose<TransposeDirection::c2f>(dataC.data(), dataF90.data(), rows, cols);
  CViewT cview(dataC.data(), rows, cols);
  F90ViewT f90view(dataF90.data(), rows, cols);

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      REQUIRE(cview(i, j) == f90view(i, j));
    }
  }

  transpose<TransposeDirection::f2c>(dataF90.data(), dataC2.data(), rows, cols);
  CViewT cview2(dataC2.data(), rows, cols);

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      REQUIRE(cview(i, j) == cview2(i, j));
    }
  }
}

TEST_CASE("transpose_3d", "math_util")
{
  using KT       = KokkosTypes<HostDevice>;
  using CViewT   = Unmanaged<typename KT::view<int***> >;
  using F90ViewT = Unmanaged<typename KT::lview<int***> >;

  const int rows   = 17;
  const int cols   = 27;
  const int slices = 7;
  const int total  = rows*cols*slices;

  std::vector<int> dataC(total), dataF90(total), dataC2(total);
  for (int i = 0; i < total; ++i) {
    dataC[i] = i;
  }

  transpose<TransposeDirection::c2f>(dataC.data(), dataF90.data(), rows, cols, slices);
  CViewT cview(dataC.data(), rows, cols, slices);
  F90ViewT f90view(dataF90.data(), rows, cols, slices);

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      for (int k = 0; k < slices; ++k) {
        REQUIRE(cview(i, j, k) == f90view(i, j, k));
      }
    }
  }

  transpose<TransposeDirection::f2c>(dataF90.data(), dataC2.data(), rows, cols, slices);
  CViewT cview2(dataC2.data(), rows, cols, slices);

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      for (int k = 0; k < slices; ++k) {
        REQUIRE(cview(i, j, k) == cview2(i, j, k));
      }
    }
  }
}

} // empty namespace
