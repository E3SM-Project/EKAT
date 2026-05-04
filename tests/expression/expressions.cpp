#include <catch2/catch.hpp>

#include "ekat_expression_eval.hpp"

#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_binary_predicate.hpp"
#include "ekat_expression_conditional.hpp"
#include "ekat_expression_math.hpp"
#include "ekat_expression_view.hpp"

#include "ekat_view_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include <random>

namespace ekat {

template<typename ViewT>
void bin_ops (const ViewT& x, const ViewT& y, const ViewT& z)
{
  const auto eps = std::numeric_limits<decltype(*z.data())>::epsilon();
  const auto tol = 1e5*eps;
  auto xe = expression(x);
  auto ye = expression(y);
  auto expression = xe*ye - 1/ye + 2*xe;

  evaluate(expression,z);

  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);
  auto zh = create_host_mirror_and_copy(z);
  for (size_t i=0; i<zh.size(); ++i) {
    auto x_val = xh.data()[i];
    auto y_val = yh.data()[i];
    auto z_val = zh.data()[i];
    auto tgt   = x_val*y_val-1/y_val+2*x_val;
    REQUIRE_THAT (z_val, Catch::Matchers::WithinRel(tgt,tol));
  }
}

template<typename ViewT>
void math_fcns (const ViewT& x, const ViewT& y, const ViewT& z)
{
  auto xe = expression(x);
  auto ye = expression(y);
  auto expression = 2*exp(-xe)*sin(xe)*log(ye)-sqrt(xe)+pow(ye,2)+pow(3,xe);

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

template<typename ViewT, typename BViewT>
void predicate (const ViewT& x, const ViewT& y, const BViewT& z)
{
  auto xe = expression(x);
  auto ye = expression(y);
  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);

  // EQ
  {
    auto expression = xe==ye;
    evaluate(expression,z);

    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = x_val==y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // NE
  {
    auto expression = xe!=ye;
    evaluate(expression,z);

    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = x_val!=y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // GT
  {
    auto expression = xe>ye;
    evaluate(expression,z);

    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = x_val>y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // GE
  {
    auto expression = xe>=ye;
    evaluate(expression,z);

    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = x_val>=y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // LT
  {
    auto expression = xe<ye;
    evaluate(expression,z);

    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = x_val<y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // LE
  {
    auto expression = xe<=ye;
    evaluate(expression,z);

    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = x_val<=y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // AND/OR
  {
    // These two should eval to exactly opposite values (due to De Morgan's laws)
    auto expr_and = xe>=0.5 && ye<=0.5;
    auto expr_or  = xe<0.5 || ye>0.5;
    auto z2 = Kokkos::create_mirror(typename BViewT::memory_space{}, z);
    evaluate(expr_and,z);
    evaluate(expr_or,z2);

    auto zh = create_host_mirror_and_copy(z);
    auto z2h = create_host_mirror_and_copy(z2);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto z2_val = z2h.data()[i];
      auto tgt   = x_val>=0.5 and y_val<=0.5;
      REQUIRE (z_val==tgt);
      REQUIRE (z2_val==(not tgt));
    }
  }
}

template<typename ViewT>
void conditionals (const ViewT& x, const ViewT& y, const ViewT& z)
{
  auto xe = expression(x);
  auto ye = expression(y);
  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);

  // All expressions
  {
    auto expression = if_then_else(sqrt(xe)>=0.5,xe+ye,xe-ye);
    evaluate(expression,z);
    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = std::sqrt(x_val)>=0.5 ? x_val+y_val : x_val-y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // cond(expr,expr,scalar)
  {
    auto expression = if_then_else(sqrt(xe)>=0.5,xe+ye,-3);
    evaluate(expression,z);
    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = std::sqrt(x_val)>=0.5 ? x_val+y_val : -3;
      REQUIRE (z_val==tgt);
    }
  }
  // cond(bool,expr,expr)
  {
    auto expression = if_then_else(false,xe+ye,xe-ye);
    evaluate(expression,z);
    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto x_val = xh.data()[i];
      auto y_val = yh.data()[i];
      auto z_val = zh.data()[i];
      auto tgt   = x_val-y_val;
      REQUIRE (z_val==tgt);
    }
  }
  // cond(bool,scalar,expr)
  {
    auto expression = if_then_else(true,42,xe-ye);
    evaluate(expression,z);
    auto zh = create_host_mirror_and_copy(z);
    for (size_t i=0; i<zh.size(); ++i) {
      auto z_val = zh.data()[i];
      auto tgt   = 42;
      REQUIRE (z_val==tgt);
    }
  }
}

TEST_CASE("expressions", "") {

  using Real = double;

  std::random_device rdev;
  const int catchRngSeed = Catch::rngSeed();
  int seed = catchRngSeed==0 ? rdev()/2 : catchRngSeed;
  std::mt19937_64 engine(seed);

  std::uniform_real_distribution<Real> pdf(0.1, 1);

  using kk_t = KokkosTypes<DefaultDevice>;
  SECTION ("0d") {
    printf("running 0d tests with rng seed: %d\n",seed);

    kk_t::view_ND<Real,0> x ("x");
    kk_t::view_ND<Real,0> y ("y");
    kk_t::view_ND<Real,0> z ("z");
    kk_t::view_ND<bool,0> zb("z");

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y,z);
    math_fcns(x,y,z);
    predicate(x,y,zb);
    conditionals(x,y,z);
  }

  SECTION ("1d") {
    printf("running 1d tests with rng seed: %d\n",seed);

    kk_t::view_1d<Real> x ("x",1000);
    kk_t::view_1d<Real> y ("y",1000);
    kk_t::view_1d<Real> z ("z",1000);
    kk_t::view_1d<bool> zb("zb",1000);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y,z);
    math_fcns(x,y,z);
    predicate(x,y,zb);
    conditionals(x,y,z);
  }

  SECTION ("2d") {
    printf("running 2d tests with rng seed: %d\n",seed);

    kk_t::view_2d<Real> x ("x",100,32);
    kk_t::view_2d<Real> y ("y",100,32);
    kk_t::view_2d<Real> z ("z",100,32);
    kk_t::view_2d<bool> zb("z",100,32);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y,z);
    math_fcns(x,y,z);
    predicate(x,y,zb);
    conditionals(x,y,z);
  }

  SECTION ("3d") {
    printf("running 3d tests with rng seed: %d\n",seed);

    kk_t::view_3d<Real> x ("x",100,4,32);
    kk_t::view_3d<Real> y ("y",100,4,32);
    kk_t::view_3d<Real> z ("z",100,4,32);
    kk_t::view_3d<bool> zb("z",100,4,32);

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y,z);
    math_fcns(x,y,z);
    predicate(x,y,zb);
    conditionals(x,y,z);
  }
}

} // namespace ekat
