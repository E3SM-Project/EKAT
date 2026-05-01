#include <catch2/catch.hpp>

#include "ekat_expression_helpers.hpp"
#include "ekat_expression_eval.hpp"

#include "ekat_view_utils.hpp"
#include "ekat_kokkos_types.hpp"

#include <random>
#include <numeric>

namespace ekat {

template<typename ViewT>
void bin_ops (const ViewT& x, const ViewT& y, const ViewT& z)
{
  auto xe = view_expression(x);
  auto ye = view_expression(y);
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
  auto xe = view_expression(x);
  auto ye = view_expression(y);
  auto expression = conditional(sqrt(xe)>=0.5,xe+ye,xe-ye);

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
    kk_t::view_ND<Real,0> z("z");

    genRandArray(x,engine,pdf);
    genRandArray(y,engine,pdf);

    bin_ops(x,y,z);
    math_fcns(x,y,z);
    conditionals(x,y,z);
  }

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

// -----------------------------------------------------------------------
// Tests for team-parallel evaluate
// -----------------------------------------------------------------------

TEST_CASE("team_evaluate", "") {
  using Real   = double;
  using kk_t   = KokkosTypes<DefaultDevice>;
  using Policy = typename kk_t::TeamPolicy;
  using Member = typename kk_t::MemberType;

  std::random_device rdev;
  const int catchRngSeed = Catch::rngSeed();
  int seed = catchRngSeed==0 ? rdev()/2 : catchRngSeed;
  std::mt19937_64 engine(seed);
  std::uniform_real_distribution<Real> pdf(0.1, 1.0);

  // Number of teams (league) and vector-lane size (team)
  const int nteams = 20;
  const int ncols  = 32;

  // -----------------------------------------------------------------------
  // Step 1: Verify expressions can be constructed on device (KOKKOS_FUNCTION)
  // -----------------------------------------------------------------------
  SECTION("device_ctor") {
    kk_t::view_1d<Real> v("v", nteams * ncols);
    kk_t::view_1d<Real> w("w", nteams * ncols);
    kk_t::view_1d<Real> z("z", nteams * ncols);

    genRandArray(v, engine, pdf);
    genRandArray(w, engine, pdf);

    // Build the expression tree INSIDE a device lambda
    Policy policy(nteams, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      int i = team.league_rank();
      // Construct subviews and expressions inside the kernel
      auto row_v = Kokkos::subview(v, Kokkos::make_pair(i*ncols, (i+1)*ncols));
      auto row_w = Kokkos::subview(w, Kokkos::make_pair(i*ncols, (i+1)*ncols));
      auto row_z = Kokkos::subview(z, Kokkos::make_pair(i*ncols, (i+1)*ncols));
      auto ve = view_expression(row_v);
      auto we = view_expression(row_w);
      auto expr = ve * we + 2*ve - we;
      evaluate(expr, team, row_z, TeamEvalMode::TeamVectorRange);
    });
    Kokkos::fence();

    auto vh = create_host_mirror_and_copy(v);
    auto wh = create_host_mirror_and_copy(w);
    auto zh = create_host_mirror_and_copy(z);
    for (int k = 0; k < nteams * ncols; ++k) {
      Real tgt = vh(k)*wh(k) + 2*vh(k) - wh(k);
      REQUIRE(zh(k) == Approx(tgt).epsilon(1e-10));
    }
  }

  // -----------------------------------------------------------------------
  // Step 3a: TeamVectorRange mode on rank-1 expressions
  // -----------------------------------------------------------------------
  SECTION("team_vector_range_1d") {
    kk_t::view_2d<Real> v("v", nteams, ncols);
    kk_t::view_2d<Real> w("w", nteams, ncols);
    kk_t::view_2d<Real> z("z", nteams, ncols);

    genRandArray(v, engine, pdf);
    genRandArray(w, engine, pdf);

    Policy policy(nteams, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      int i = team.league_rank();
      auto row_v = Kokkos::subview(v, i, Kokkos::ALL());
      auto row_w = Kokkos::subview(w, i, Kokkos::ALL());
      auto row_z = Kokkos::subview(z, i, Kokkos::ALL());
      auto expr = view_expression(row_v) * view_expression(row_w) + 1.0;
      evaluate(expr, team, row_z, TeamEvalMode::TeamVectorRange);
    });
    Kokkos::fence();

    auto vh = create_host_mirror_and_copy(v);
    auto wh = create_host_mirror_and_copy(w);
    auto zh = create_host_mirror_and_copy(z);
    for (int i = 0; i < nteams; ++i)
      for (int j = 0; j < ncols; ++j)
        REQUIRE(zh(i,j) == Approx(vh(i,j)*wh(i,j) + 1.0).epsilon(1e-10));
  }

  // -----------------------------------------------------------------------
  // Step 3b: ThreadVectorRange mode (caller inside TeamThreadRange)
  // -----------------------------------------------------------------------
  SECTION("thread_vector_range_1d") {
    const int nrows = 4;
    kk_t::view_3d<Real> v("v", nteams, nrows, ncols);
    kk_t::view_3d<Real> w("w", nteams, nrows, ncols);
    kk_t::view_3d<Real> z("z", nteams, nrows, ncols);

    genRandArray(v, engine, pdf);
    genRandArray(w, engine, pdf);

    Policy policy(nteams, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      int i = team.league_rank();
      Kokkos::parallel_for(Kokkos::TeamThreadRange(team, nrows), [&](int r) {
        auto row_v = Kokkos::subview(v, i, r, Kokkos::ALL());
        auto row_w = Kokkos::subview(w, i, r, Kokkos::ALL());
        auto row_z = Kokkos::subview(z, i, r, Kokkos::ALL());
        auto expr = view_expression(row_v) - view_expression(row_w);
        evaluate(expr, team, row_z, TeamEvalMode::ThreadVectorRange);
      });
    });
    Kokkos::fence();

    auto vh = create_host_mirror_and_copy(v);
    auto wh = create_host_mirror_and_copy(w);
    auto zh = create_host_mirror_and_copy(z);
    for (int i = 0; i < nteams; ++i)
      for (int r = 0; r < nrows; ++r)
        for (int j = 0; j < ncols; ++j)
          REQUIRE(zh(i,r,j) == Approx(vh(i,r,j) - wh(i,r,j)).epsilon(1e-10));
  }

  // -----------------------------------------------------------------------
  // Step 3c: TeamThread_ThreadVector mode on rank-2 expressions
  // -----------------------------------------------------------------------
  SECTION("team_thread_threadvector_2d") {
    const int nrows = 4;
    kk_t::view_3d<Real> v("v", nteams, nrows, ncols);
    kk_t::view_3d<Real> w("w", nteams, nrows, ncols);
    kk_t::view_3d<Real> z("z", nteams, nrows, ncols);

    genRandArray(v, engine, pdf);
    genRandArray(w, engine, pdf);

    Policy policy(nteams, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      int i = team.league_rank();
      auto slab_v = Kokkos::subview(v, i, Kokkos::ALL(), Kokkos::ALL());
      auto slab_w = Kokkos::subview(w, i, Kokkos::ALL(), Kokkos::ALL());
      auto slab_z = Kokkos::subview(z, i, Kokkos::ALL(), Kokkos::ALL());
      auto expr = view_expression(slab_v) + view_expression(slab_w);
      evaluate(expr, team, slab_z, TeamEvalMode::TeamThread_ThreadVector);
    });
    Kokkos::fence();

    auto vh = create_host_mirror_and_copy(v);
    auto wh = create_host_mirror_and_copy(w);
    auto zh = create_host_mirror_and_copy(z);
    for (int i = 0; i < nteams; ++i)
      for (int r = 0; r < nrows; ++r)
        for (int j = 0; j < ncols; ++j)
          REQUIRE(zh(i,r,j) == Approx(vh(i,r,j) + wh(i,r,j)).epsilon(1e-10));
  }

  // -----------------------------------------------------------------------
  // Step 4a: ReduceExpression in isolation
  // -----------------------------------------------------------------------
  SECTION("reduce_sum_isolation") {
    kk_t::view_2d<Real> v("v", nteams, ncols);
    genRandArray(v, engine, pdf);

    // One reduction result slot per team
    Kokkos::View<Real*> sums("sums", nteams);

    // Use a rank-0 result view to hold outputs
    kk_t::view_ND<Real,0> result("result");

    // Compute sum of each row using reduce_sum inside a team kernel
    // then store the result for later verification.
    kk_t::view_1d<Real> team_sums("team_sums", nteams);
    Kokkos::View<Real*> reduce_store("reduce_store", nteams);

    Policy policy(nteams, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      int i = team.league_rank();
      auto row = Kokkos::subview(v, i, Kokkos::ALL());
      auto expr = reduce_sum(view_expression(row), reduce_store);
      // Only setup is needed; eval() returns the scalar
      expr.setup(team);
      Kokkos::single(Kokkos::PerTeam(team), [&]() {
        team_sums(i) = expr.eval();
      });
    });
    Kokkos::fence();

    // Verify against host computation
    auto vh = create_host_mirror_and_copy(v);
    auto sh = create_host_mirror_and_copy(team_sums);
    for (int i = 0; i < nteams; ++i) {
      Real expected = 0.0;
      for (int j = 0; j < ncols; ++j) expected += vh(i,j);
      REQUIRE(sh(i) == Approx(expected).epsilon(1e-8));
    }
  }

  // -----------------------------------------------------------------------
  // Step 4b: ReduceExpression composed in a larger expression
  //   expr(j) = reduce_sum(row_v) * row_w(j) + row_v(j)
  // -----------------------------------------------------------------------
  SECTION("reduce_sum_composed") {
    kk_t::view_2d<Real> v("v", nteams, ncols);
    kk_t::view_2d<Real> w("w", nteams, ncols);
    kk_t::view_2d<Real> z("z", nteams, ncols);

    genRandArray(v, engine, pdf);
    genRandArray(w, engine, pdf);

    Kokkos::View<Real*> reduce_store("reduce_store", nteams);

    Policy policy(nteams, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      int i = team.league_rank();
      auto row_v = Kokkos::subview(v, i, Kokkos::ALL());
      auto row_w = Kokkos::subview(w, i, Kokkos::ALL());
      auto row_z = Kokkos::subview(z, i, Kokkos::ALL());
      // reduce_sum(v) is rank-0; row_w is rank-1 → result rank-1
      auto expr = reduce_sum(view_expression(row_v), reduce_store)
                  * view_expression(row_w)
                  + view_expression(row_v);
      evaluate(expr, team, row_z);
    });
    Kokkos::fence();

    auto vh = create_host_mirror_and_copy(v);
    auto wh = create_host_mirror_and_copy(w);
    auto zh = create_host_mirror_and_copy(z);
    for (int i = 0; i < nteams; ++i) {
      Real sum_v = 0.0;
      for (int j = 0; j < ncols; ++j) sum_v += vh(i,j);
      for (int j = 0; j < ncols; ++j) {
        Real tgt = sum_v * wh(i,j) + vh(i,j);
        REQUIRE(zh(i,j) == Approx(tgt).epsilon(1e-8));
      }
    }
  }

  // -----------------------------------------------------------------------
  // Step 4c: reduce_max and reduce_min
  // -----------------------------------------------------------------------
  SECTION("reduce_max_min") {
    kk_t::view_2d<Real> v("v", nteams, ncols);
    kk_t::view_2d<Real> z_max("z_max", nteams, ncols);
    kk_t::view_2d<Real> z_min("z_min", nteams, ncols);

    genRandArray(v, engine, pdf);

    Kokkos::View<Real*> store_max("store_max", nteams);
    Kokkos::View<Real*> store_min("store_min", nteams);

    Policy policy(nteams, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      int i = team.league_rank();
      auto row_v   = Kokkos::subview(v,     i, Kokkos::ALL());
      auto row_zmax = Kokkos::subview(z_max, i, Kokkos::ALL());
      auto row_zmin = Kokkos::subview(z_min, i, Kokkos::ALL());
      auto ve = view_expression(row_v);
      // Scale each element by max and min of the same row
      auto expr_max = reduce_max(ve, store_max) * ve;
      auto expr_min = reduce_min(ve, store_min) * ve;
      evaluate(expr_max, team, row_zmax);
      evaluate(expr_min, team, row_zmin);
    });
    Kokkos::fence();

    auto vh    = create_host_mirror_and_copy(v);
    auto zh_mx = create_host_mirror_and_copy(z_max);
    auto zh_mn = create_host_mirror_and_copy(z_min);
    for (int i = 0; i < nteams; ++i) {
      Real mx = vh(i,0), mn = vh(i,0);
      for (int j = 1; j < ncols; ++j) {
        mx = std::max(mx, vh(i,j));
        mn = std::min(mn, vh(i,j));
      }
      for (int j = 0; j < ncols; ++j) {
        REQUIRE(zh_mx(i,j) == Approx(mx * vh(i,j)).epsilon(1e-10));
        REQUIRE(zh_mn(i,j) == Approx(mn * vh(i,j)).epsilon(1e-10));
      }
    }
  }

  // -----------------------------------------------------------------------
  // Step 4d: reduce_all and reduce_any (logical reductions)
  // -----------------------------------------------------------------------
  SECTION("reduce_all_any") {
    // Use a view of int (0 or 1) so LAnd/LOr work naturally
    using Int = int;
    kk_t::view_1d<Int> flags("flags", ncols);
    kk_t::view_1d<Int> z("z", ncols);
    kk_t::view_ND<Int,0> all_result("all_result");
    kk_t::view_ND<Int,0> any_result("any_result");

    // Fill: first half 1, second half 0
    auto fh = Kokkos::create_mirror_view(flags);
    for (int j = 0; j < ncols; ++j) fh(j) = (j < ncols/2) ? 1 : 0;
    Kokkos::deep_copy(flags, fh);

    Kokkos::View<Int*> store_all("store_all", 1);
    Kokkos::View<Int*> store_any("store_any", 1);

    // Run in a single-team kernel (league_size=1)
    Policy policy(1, 1);
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const Member& team) {
      auto fe = view_expression(flags);
      auto all_expr = reduce_all(fe, store_all);
      auto any_expr = reduce_any(fe, store_any);
      all_expr.setup(team);
      any_expr.setup(team);
      Kokkos::single(Kokkos::PerTeam(team), [&]() {
        all_result() = all_expr.eval();  // false because second half is 0
        any_result() = any_expr.eval();  // true  because first half is 1
      });
    });
    Kokkos::fence();

    auto all_h = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace{}, all_result);
    auto any_h = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace{}, any_result);
    REQUIRE(all_h() == 0);   // not all true
    REQUIRE(any_h() != 0);   // at least one true
  }
}

} // namespace ekat

