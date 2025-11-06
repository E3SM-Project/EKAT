#include <catch2/catch.hpp>

#include "ekat_expression_view.hpp"
#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_compare.hpp"
#include "ekat_expression_conditional.hpp"
#include "ekat_expression_math.hpp"
#include "ekat_expression_eval.hpp"
#include "ekat_expression_base.hpp"

#include "ekat_pack_utils.hpp"
#include "ekat_pack_kokkos.hpp"
#include "ekat_pack.hpp"
#include "ekat_view_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include "ekat_test_config.h"

#include <random>

namespace ekat {

template<typename ViewT>
void bin_ops (const ViewT& x, const ViewT& y, const ViewT& z)
{
  auto xe = view_expression(x);
  auto ye = view_expression(y);
  auto expression = xe*ye - 1/ye + 2*xe;

  evaluate(expression,z);

  auto xh = create_host_mirror_and_copy(scalarize(x));
  auto yh = create_host_mirror_and_copy(scalarize(y));
  auto zh = create_host_mirror_and_copy(scalarize(z));
  for (size_t i=0; i<zh.size(); ++i) {
    auto x_val = xh.data()[i];
    auto y_val = yh.data()[i];
    auto z_val = zh.data()[i];
    auto tgt   = x_val*y_val-1/y_val+2*x_val;
    REQUIRE (z_val==Approx(tgt).epsilon(1e-10));
  }
}

template<typename ViewT>
void math_fcns (const ViewT& x, const ViewT& y, const ViewT& z)
{
  auto xe = view_expression(x);
  auto ye = view_expression(y);
  auto expression = 2*exp(-xe)*sin(xe)*log(ye)-sqrt(xe)+pow(ye,2)+pow(3,xe);

  evaluate(expression,z);

  auto xh = create_host_mirror_and_copy(scalarize(x));
  auto yh = create_host_mirror_and_copy(scalarize(y));
  auto zh = create_host_mirror_and_copy(scalarize(z));
  for (size_t i=0; i<zh.size(); ++i) {
    auto x_val = xh.data()[i];
    auto y_val = yh.data()[i];
    auto z_val = zh.data()[i];
    auto tgt   = 2*std::exp(-x_val)*std::sin(x_val)*std::log(y_val)
               - std::sqrt(x_val) + std::pow(y_val,2) + std::pow(3,x_val);
    REQUIRE (z_val==Approx(tgt).epsilon(1e-10));
  }
}

template<typename ViewT>
void conditionals (const ViewT& x, const ViewT& y, const ViewT& z)
{
  auto xe = view_expression(x);
  auto ye = view_expression(y);
  auto expression = conditional(sqrt(xe)>=0.5,xe+ye,xe-ye);

  evaluate(expression,z);

  auto xh = create_host_mirror_and_copy(scalarize(x));
  auto yh = create_host_mirror_and_copy(scalarize(y));
  auto zh = create_host_mirror_and_copy(scalarize(z));
  for (size_t i=0; i<zh.size(); ++i) {
    auto x_val = xh.data()[i];
    auto y_val = yh.data()[i];
    auto z_val = zh.data()[i];
    auto tgt   = std::sqrt(x_val)>=0.5 ? x_val+y_val : x_val-y_val;
    REQUIRE (z_val==tgt);
  }
}

TEST_CASE("expressions", "") {

  using Real = double;
  using PackT = Pack<Real,EKAT_TEST_PACK_SIZE>;

  std::random_device rdev;
  const int catchRngSeed = Catch::rngSeed();
  int seed = catchRngSeed==0 ? rdev()/2 : catchRngSeed;
  std::mt19937_64 engine(seed);
  std::uniform_real_distribution<Real> pdf(0.1, 1);

  using kk_t = KokkosTypes<DefaultDevice>;
  SECTION ("1d") {
    int dim0 = PackInfo<EKAT_TEST_PACK_SIZE>::num_packs(1000);
    kk_t::view_1d<PackT> x("x",dim0);
    kk_t::view_1d<PackT> y("y",dim0);
    kk_t::view_1d<PackT> z("z",dim0);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);
    
    bin_ops(x,y,z);
    math_fcns(x,y,z);
    conditionals(x,y,z);
  }

  SECTION ("2d") {
    int dim0 = 100;
    int dim1 = PackInfo<EKAT_TEST_PACK_SIZE>::num_packs(1000);
    kk_t::view_2d<PackT> x("x",dim0,dim1);
    kk_t::view_2d<PackT> y("y",dim0,dim1);
    kk_t::view_2d<PackT> z("z",dim0,dim1);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);
    
    bin_ops(x,y,z);
    math_fcns(x,y,z);
    conditionals(x,y,z);
  }

  SECTION ("3d") {
    int dim0 = 100;
    int dim1 = 4;
    int dim2 = PackInfo<EKAT_TEST_PACK_SIZE>::num_packs(32);
    kk_t::view_3d<PackT> x("x",dim0,dim1,dim2);
    kk_t::view_3d<PackT> y("y",dim0,dim1,dim2);
    kk_t::view_3d<PackT> z("z",dim0,dim1,dim2);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);
    
    bin_ops(x,y,z);
    math_fcns(x,y,z);
    conditionals(x,y,z);
  }
}

} // namespace ekat
