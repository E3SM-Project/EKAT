#include <catch2/catch.hpp>

#include "ekat_expression_eval.hpp"

#include "ekat_expression_binary_op.hpp"
#include "ekat_expression_binary_predicate.hpp"
#include "ekat_expression_conditional.hpp"
#include "ekat_expression_math.hpp"
#include "ekat_expression_view.hpp"

#include "ekat_view_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include "ekat_test_config.h"

#include <random>

namespace ekat {

using kk_t = KokkosTypes<DefaultDevice>;
using member_t = typename kk_t::MemberType;
using exec_space = typename kk_t::ExeSpace;
using Policy1D = Kokkos::RangePolicy<exec_space>;
template<int N>
using PolicyMD = Kokkos::MDRangePolicy<exec_space,Kokkos::Rank<N>>;

template<typename ViewT>
void bin_ops (const ViewT& x, const ViewT& y, const ViewT& z)
{
  constexpr int N = ViewT::rank;

  auto eps = std::numeric_limits<Real>::epsilon();
  auto tol = 1e5*eps;

  auto eval = KOKKOS_LAMBDA (auto... args) {
    auto xe = expression(x);
    auto ye = expression(y);
    auto ze = xe*ye - 1/ye + 2*xe;
    z(args...) = ze.eval(eval_specs(),args...);
  };

  int beg[N==0 ? 1 : N] = {};
  int end[N==0 ? 1 : N] = {};
  for (int i=0; i<N; ++i) {
    EKAT_REQUIRE_MSG (e.extent(i)==result.extent_int(i),
      "[evaluate] Error! Input expression and output view have incompatible extents.\n");
    end[i] = z.extent(i);
  }

  if constexpr (N==0) {
    Policy1D p(0,1);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==1) {
    Policy1D p(0,end[0]);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==2) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==3) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==4) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==5) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==6) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==7) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  }

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
  constexpr int N = ViewT::rank;

  auto eps = std::numeric_limits<Real>::epsilon();
  auto tol = 1e5*eps;

  auto eval = KOKKOS_LAMBDA (auto... args) {
    auto xe = expression(x);
    auto ye = expression(y);
    auto ze = 2*exp(-xe)*sin(xe)*log(ye)-sqrt(xe)+pow(ye,2)+pow(3,xe);
    z(args...) = ze.eval(eval_specs(),args...);
  };

  int beg[N==0 ? 1 : N] = {};
  int end[N==0 ? 1 : N] = {};
  for (int i=0; i<N; ++i) {
    EKAT_REQUIRE_MSG (e.extent(i)==result.extent_int(i),
      "[evaluate] Error! Input expression and output view have incompatible extents.\n");
    end[i] = z.extent(i);
  }

  if constexpr (N==0) {
    Policy1D p(0,1);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==1) {
    Policy1D p(0,end[0]);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==2) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==3) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==4) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==5) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==6) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==7) {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  } else {
    PolicyMD p(beg,end);
    Kokkos::parallel_for(p,eval);
  }

  auto xh = create_host_mirror_and_copy(x);
  auto yh = create_host_mirror_and_copy(y);
  auto zh = create_host_mirror_and_copy(z);
  for (size_t i=0; i<zh.size(); ++i) {
    auto x_val = xh.data()[i];
    auto y_val = yh.data()[i];
    auto z_val = zh.data()[i];
    auto tgt   = 2*std::exp(-x_val)*std::sin(x_val)*std::log(y_val)
               - std::sqrt(x_val) + std::pow(y_val,2) + std::pow(3,x_val);
    REQUIRE_THAT (z_val, Catch::Matchers::WithinRel(tgt,tol));
  }
}

TEST_CASE("team_expressions") {

  std::random_device rdev;
  const int catchRngSeed = Catch::rngSeed();
  int seed = catchRngSeed==0 ? rdev()/2 : catchRngSeed;
  std::mt19937_64 engine(seed);

  std::uniform_real_distribution<Real> pdf(0.1, 1);

  using kk_t = KokkosTypes<DefaultDevice>;

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
  }
}

} // namespace ekat

