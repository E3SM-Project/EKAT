#include <catch2/catch.hpp>

#include "ekat_expression_view.hpp"
#include "ekat_expression_assignment_op.hpp"
#include "ekat_expression_compare.hpp"
#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_conditional.hpp"
#include "ekat_expression_math.hpp"
#include "ekat_expression_eval.hpp"
#include "ekat_expression_base.hpp"

#include "ekat_view_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include <random>

namespace ekat {

template<typename ViewT>
void bin_ops (const ViewT& x, const ViewT& y)
{
  ViewT y0("",y.extent(0),y.extent(1),y.extent(2));
  Kokkos::deep_copy(y0,y);

  auto xe = view_expression(x);
  auto ye = view_expression(y);
  auto expression = (ye*=2*xe)+=xe/2;

  evaluate(expression);

  auto xh  = create_host_mirror_and_copy(x);
  auto y0h = create_host_mirror_and_copy(y0);
  auto yh  = create_host_mirror_and_copy(y);
  for (size_t i=0; i<yh.size(); ++i) {
    auto x0_val = xh.data()[i];
    auto y0_val = y0h.data()[i];
    auto y_val  = yh.data()[i];
    auto tgt   = 2*x0_val*y0_val+x0_val/2;
    REQUIRE (y_val==Approx(tgt).epsilon(1e-10));
  }
}

template<typename ViewT>
void conditionals (const ViewT& x, const ViewT& y)
{
  ViewT x0("",x.extent(0),x.extent(1),x.extent(2));
  ViewT y0("",y.extent(0),y.extent(1),y.extent(2));
  Kokkos::deep_copy(x0,x);
  Kokkos::deep_copy(y0,y);

  auto xe = view_expression(x);
  auto ye = view_expression(y);
  auto expression = conditional(sqrt(xe)>=0.5,xe,ye) *= 2;

  evaluate(expression);

  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);
  auto x0h = create_host_mirror_and_copy(x0);
  auto y0h = create_host_mirror_and_copy(y0);
  for (size_t i=0; i<xh.size(); ++i) {
    auto x_val = xh.data()[i];
    auto y_val = yh.data()[i];
    auto x0_val = x0h.data()[i];
    auto y0_val = y0h.data()[i];
    if (std::sqrt(x0_val)>=0.5) {
      REQUIRE (x_val==(x0_val*2));
    } else {
      REQUIRE (y_val==(y0_val*2));
    }
  }
}

TEST_CASE("assignment_expressions", "") {

  using Real = double;

  std::random_device rdev;
  const int catchRngSeed = Catch::rngSeed();
  int seed = catchRngSeed==0 ? rdev()/2 : catchRngSeed;
  std::mt19937_64 engine(seed);

  std::uniform_real_distribution<Real> pdf(0.1, 1);

  using kk_t = KokkosTypes<DefaultDevice>;
  SECTION ("0d") {
    kk_t::view_ND<Real,0> x("x");
    kk_t::view_ND<Real,0> y("y");

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y);
    conditionals(x,y);
  }

  SECTION ("1d") {
    kk_t::view_1d<Real> x("x",1000);
    kk_t::view_1d<Real> y("y",1000);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y);
    conditionals(x,y);
  }

  SECTION ("2d") {
    kk_t::view_2d<Real> x("x",100,32);
    kk_t::view_2d<Real> y("y",100,32);
    kk_t::view_2d<Real> z("z",100,32);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y);
    conditionals(x,y);
  }

  SECTION ("3d") {
    kk_t::view_3d<Real> x("x",100,4,32);
    kk_t::view_3d<Real> y("y",100,4,32);
    kk_t::view_3d<Real> z("z",100,4,32);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y);
    conditionals(x,y);
  }
}

} // namespace ekat

