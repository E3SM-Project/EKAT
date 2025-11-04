#include <catch2/catch.hpp>

#include "ekat_expression_view.hpp"
#include "ekat_expression_conditional.hpp"
#include "ekat_expression_helpers.hpp"
#include "ekat_expression_eval.hpp"
#include "ekat_expression_base.hpp"

#include "ekat_view_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include <random>

namespace ekat {

template<typename ViewT>
void bin_ops (const ViewT& x, const ViewT& y, const ViewT& z)
{
  auto expression = x*y - 1/y + 2*x;

  evaluate(expression,z);

  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);
  auto zh = create_host_mirror_and_copy(z);
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
  auto expression = 2*exp(-x)*sin(x)*log(y)-sqrt(x)+pow(y,2)+pow(3,x);

  evaluate(expression,z);

  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);
  auto zh = create_host_mirror_and_copy(z);
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
  auto expression = conditional(sqrt(x)>=0.5,x+y,x-y);

  evaluate(expression,z);

  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);
  auto zh = create_host_mirror_and_copy(z);
  for (size_t i=0; i<zh.size(); ++i) {
    auto x_val = xh.data()[i];
    auto y_val = yh.data()[i];
    auto z_val = zh.data()[i];
    auto tgt   = std::sqrt(x_val)>=0.5 ? x_val+y_val : x_val-y_val;
    REQUIRE (z_val==tgt);
  }
}

TEST_CASE("expressions", "") {

  std::random_device rdev;
  const int catchRngSeed = Catch::rngSeed();
  int seed = catchRngSeed==0 ? rdev()/2 : catchRngSeed;
  std::mt19937_64 engine(seed);

  std::uniform_real_distribution<Real> pdf(0.1, 1);

  using kk_t = KokkosTypes<DefaultDevice>;
  SECTION ("1d") {
    kk_t::view_1d<Real> x("x",1000);
    kk_t::view_1d<Real> y("y",1000);
    kk_t::view_1d<Real> z("z",1000);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);
    
    bin_ops(x,y,z);
    math_fcns(x,y,z);
    conditionals(x,y,z);
  }

  SECTION ("2d") {
    kk_t::view_2d<Real> x("x",100,32);
    kk_t::view_2d<Real> y("y",100,32);
    kk_t::view_2d<Real> z("z",100,32);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);
    
    bin_ops(x,y,z);
    math_fcns(x,y,z);
    conditionals(x,y,z);
  }

  SECTION ("3d") {
    kk_t::view_3d<Real> x("x",100,4,32);
    kk_t::view_3d<Real> y("y",100,4,32);
    kk_t::view_3d<Real> z("z",100,4,32);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);
    
    bin_ops(x,y,z);
    math_fcns(x,y,z);
    conditionals(x,y,z);
  }
}

} // namespace ekat
