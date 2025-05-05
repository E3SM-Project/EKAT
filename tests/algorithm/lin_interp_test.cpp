#include <catch2/catch.hpp>

#include "ekat_lin_interp.hpp"
#include "ekat_test_utils.hpp"
#include "ekat_subview_utils.hpp"
#include "ekat_test_config.h"

#include <random>
#include <vector>
#include <algorithm>

namespace {

using vector_2d_t = std::vector<std::vector<Real> >;

template <typename Scalar, typename PDF>
void populate_array(int length, Scalar* x,
                    std::default_random_engine& generator,
                    PDF&& pdf,
                    const bool sort)
{
  for (int j = 0; j < length; ++j)
    x[j] = pdf(generator);

  if (sort)
    std::sort(x, x + length);
}

template <typename Scalar>
void populate_li_input(int km1, int km2, Scalar* x1_i, Scalar* y1_i, Scalar* x2_i, std::default_random_engine& generator)
{
  std::uniform_real_distribution<Real> x_dist(0.0,1.0);
  std::uniform_real_distribution<Real> y_dist(0.0,100.0);

  populate_array(km1,x1_i,generator,x_dist,true);
  populate_array(km1,y1_i,generator,y_dist,false);
  populate_array(km2,x2_i,generator,x_dist,true);
}

// Helper function, to get scalarized subview
template<typename ViewT>
auto get_col (const ViewT& packed_view, int i) ->
  decltype(ekat::scalarize(ekat::subview(packed_view,i))) {
    return ekat::scalarize(ekat::subview(packed_view,i));
}

TEST_CASE("lin_interp_api", "lin_interp")
{
  // Test if API is flexible enough to handle various combinations
  // of const and non-const inputs

  const int ncol = 0;
  const int km1 = 0;
  const int km2 = 0;

  using LIV = ekat::LinInterp<Real,EKAT_TEST_PACK_SIZE>;
  using Pack = ekat::Pack<Real,EKAT_TEST_PACK_SIZE>;
  LIV vect(ncol, km1, km2);
  const int km1_pack = ekat::npack<Pack>(km1);
  const int km2_pack = ekat::npack<Pack>(km2);

  using view_1d = typename LIV::template view_1d<Pack>;
  using view_1dc = typename LIV::template view_1d<const Pack>;

  view_1d
    x1("x1", km1_pack),
    x2("x2", km2_pack),
    y1("y1", km1_pack),
    y2("y2", km2_pack);

  view_1dc x1c(x1), x2c(x2), y1c(y1);

  Kokkos::parallel_for("lin-interp-ut-vect",
                       vect.policy(),
                       KOKKOS_LAMBDA(typename LIV::MemberType const& team_member)
  {
    vect.setup(team_member, x1, x2);
    vect.setup(team_member, x1, x2c);
    vect.setup(team_member, x1c, x2);
    vect.setup(team_member, x1c, x2c);

    vect.lin_interp(team_member, x1, x2, y1, y2);
    vect.lin_interp(team_member, x1, x2c, y1, y2);
    vect.lin_interp(team_member, x1c, x2, y1, y2);
    vect.lin_interp(team_member, x1c, x2c, y1, y2);

    vect.lin_interp(team_member, x1, x2, y1c, y2);
    vect.lin_interp(team_member, x1, x2c, y1c, y2);
    vect.lin_interp(team_member, x1c, x2, y1c, y2);
    vect.lin_interp(team_member, x1c, x2c, y1c, y2);
  });
}

TEST_CASE("lin_interp_identity", "lin_interp") {
  using LIV = ekat::LinInterp<Real,EKAT_TEST_PACK_SIZE>;
  using Pack = ekat::Pack<Real,EKAT_TEST_PACK_SIZE>;
  using packed_view_2d = typename LIV::template view_2d<Pack>;
  using real_pdf = std::uniform_real_distribution<Real>;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> k_dist(10,100);
  const int ncol = 10;

  real_pdf x_dist(0.0,1.0);
  real_pdf y_dist(0.0,100.0);

  // increase iterations for a more-thorough testing
  for (int r = 0; r < 100; ++r) {
    const int km1 = k_dist(generator);
    const int km2 = km1;

    // Views for testing TeamVectorRange
    LIV vect(ncol, km1, km2);
    const int km1_pack = ekat::npack<Pack>(km1);
    const int km2_pack = ekat::npack<Pack>(km2);
    packed_view_2d
      x1_d("x1", ncol, km1_pack),
      x2_d("x2", ncol, km2_pack),
      y1_d("y1", ncol, km1_pack),
      y2_d("y2", ncol, km2_pack);

    // Initialize kokkos packed inputs
    auto x1_h = Kokkos::create_mirror_view(x1_d);
    auto x2_h = Kokkos::create_mirror_view(x2_d);
    auto y1_h = Kokkos::create_mirror_view(y1_d);
    auto y2_h = Kokkos::create_mirror_view(y2_d);

    for (int i = 0; i < ncol; ++i) {
      auto x1s = get_col(x1_h,i);
      auto y1s = get_col(y1_h,i);
      populate_array (km1,x1s.data(),generator,x_dist,true);
      populate_array (km1,y1s.data(),generator,y_dist,false);
    }
    Kokkos::deep_copy(x2_h, x1_h); // Force x2==x1

    Kokkos::deep_copy(x1_d, x1_h);
    Kokkos::deep_copy(y1_d, y1_h);
    Kokkos::deep_copy(x2_d, x2_h);

    // Run LiVect TeamVectorRange
    Kokkos::parallel_for("lin-interp-ut-vect-ttr",
                         vect.policy(),
                         KOKKOS_LAMBDA(typename LIV::MemberType const& team_member) {
      const int i = team_member.league_rank();
      vect.setup(team_member,
                 ekat::subview(x1_d, i),
                 ekat::subview(x2_d, i));
      team_member.team_barrier();
      vect.lin_interp(team_member,
                      ekat::subview(x1_d, i),
                      ekat::subview(x2_d, i),
                      ekat::subview(y1_d, i),
                      ekat::subview(y2_d, i));
    });

    // Compare results
    Kokkos::deep_copy(y2_h, y2_d);
    auto y2_h_s = ekat::scalarize(y2_h);
    auto y1_h_s = ekat::scalarize(y1_h);
    for (int i = 0; i < ncol; ++i) {
      for (int j = 0; j < km2; ++j) {
        REQUIRE ( y2_h_s(i,j)==y1_h_s(i,j) );
      }
    }
  }
}

TEST_CASE("lin_interp_avg", "lin_interp") {
  using LIV = ekat::LinInterp<Real,EKAT_TEST_PACK_SIZE>;
  using Pack = ekat::Pack<Real,EKAT_TEST_PACK_SIZE>;
  using packed_view_2d = typename LIV::template view_2d<Pack>;
  using real_pdf = std::uniform_real_distribution<Real>;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> k_dist(10,100);
  const int ncol = 10;

  // Generate increments for x1/y1, so that we get fairly
  // spread out grids/values. This will reduce the chance
  // of cancellations in the LinInterp impl, so that we
  // can easily compare against the exact value for y2.
  real_pdf dx_dist(0.1,1.0);
  real_pdf dy_dist(0.1,1.0);

  constexpr Real tol = std::numeric_limits<Real>::epsilon()*10;
  // increase iterations for a more-thorough testing
  for (int r = 0; r < 100; ++r) {
    const int km1 = k_dist(generator);
    const int km2 = km1-1;

    // Views for testing TeamVectorRange
    LIV vect(ncol, km1, km2);
    const int km1_pack = ekat::npack<Pack>(km1);
    const int km2_pack = ekat::npack<Pack>(km2);
    packed_view_2d
      x1_d("x1", ncol, km1_pack),
      x2_d("x2", ncol, km2_pack),
      y1_d("y1", ncol, km1_pack),
      y2_d("y2", ncol, km2_pack);

    // Initialize kokkos packed inputs
    auto x1_h = Kokkos::create_mirror_view(x1_d);
    auto x2_h = Kokkos::create_mirror_view(x2_d);
    auto y1_h = Kokkos::create_mirror_view(y1_d);
    auto y2_h = Kokkos::create_mirror_view(y2_d);

    for (int i = 0; i < ncol; ++i) {
      // Do a scan sum of x1 and y1.
      auto x1s = get_col(x1_h,i);
      auto y1s = get_col(y1_h,i);
      populate_array (km1,x1s.data(),generator,dx_dist,true);
      populate_array (km1,y1s.data(),generator,dy_dist,true);
      for (int k=1; k<km1; ++k) {
        x1s[k] = x1s[k-1] + x1s[k];
        y1s[k] = y1s[k-1] + y1s[k];
      }

      // Force x2 = (x1(:,1:end-1) + x1(:2:end)) / 2
      auto x2s = get_col(x2_h,i);
      for (int k=0; k<km2; ++k) {
        x2s(k) = (x1s(k) + x1s(k+1)) / 2;
      }
    }
    Kokkos::deep_copy(x1_d, x1_h);
    Kokkos::deep_copy(y1_d, y1_h);
    Kokkos::deep_copy(x2_d, x2_h);

    // Run LiVect TeamVectorRange
    Kokkos::parallel_for("lin-interp-ut-vect-ttr",
                         vect.policy(),
                         KOKKOS_LAMBDA(typename LIV::MemberType const& team_member) {
      const int i = team_member.league_rank();
      vect.setup(team_member,
                 ekat::subview(x1_d, i),
                 ekat::subview(x2_d, i));
      team_member.team_barrier();
      vect.lin_interp(team_member,
                      ekat::subview(x1_d, i),
                      ekat::subview(x2_d, i),
                      ekat::subview(y1_d, i),
                      ekat::subview(y2_d, i));
    });

    // Compare results
    Kokkos::deep_copy(y2_h, y2_d);
    auto y2_h_s = ekat::scalarize(y2_h);
    auto y1_h_s = ekat::scalarize(y1_h);
    auto x2_h_s = ekat::scalarize(x2_h);
    auto x1_h_s = ekat::scalarize(x1_h);
    using Catch::Detail::Approx;
    for (int i = 0; i < ncol; ++i) {
      for (int j = 0; j < km2; ++j) {
        auto target = Approx( (y1_h_s(i,j)+y1_h_s(i,j+1))/2 ).epsilon(tol).margin(10*tol);
        REQUIRE ( y2_h_s(i,j)==target );
      }
    }
  }
}

TEST_CASE("lin_interp_down_sampling", "lin_interp") {
  using LIV = ekat::LinInterp<Real,EKAT_TEST_PACK_SIZE>;
  using Pack = ekat::Pack<Real,EKAT_TEST_PACK_SIZE>;
  using packed_view_2d = typename LIV::template view_2d<Pack>;
  using real_pdf = std::uniform_real_distribution<Real>;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> k_dist(10,100);
  const int ncol = 10;

  real_pdf x_dist(0.0,1.0);
  real_pdf y_dist(0.0,100.0);

  // increase iterations for a more-thorough testing
  for (int r = 0; r < 100; ++r) {
    const int km2 = k_dist(generator);
    const int km1 = km2*2;

    // Views for testing TeamVectorRange
    LIV vect(ncol, km1, km2);
    const int km1_pack = ekat::npack<Pack>(km1);
    const int km2_pack = ekat::npack<Pack>(km2);
    packed_view_2d
      x1_d("x1", ncol, km1_pack),
      x2_d("x2", ncol, km2_pack),
      y1_d("y1", ncol, km1_pack),
      y2_d("y2", ncol, km2_pack);

    // Initialize kokkos packed inputs
    auto x1_h = Kokkos::create_mirror_view(x1_d);
    auto x2_h = Kokkos::create_mirror_view(x2_d);
    auto y1_h = Kokkos::create_mirror_view(y1_d);
    auto y2_h = Kokkos::create_mirror_view(y2_d);

    for (int i = 0; i < ncol; ++i) {
      populate_array (km1,get_col(x1_h,i).data(),generator,x_dist,true);
      populate_array (km1,get_col(y1_h,i).data(),generator,y_dist,false);

      // Force x2 = x1(:,1:2:end)
      auto x1s = get_col(x1_h,i);
      auto x2s = get_col(x2_h,i);
      for (int k=0; k<km2; ++k) {
        x2s(k) = x1s(2*k);
      }
    }
    Kokkos::deep_copy(x1_d, x1_h);
    Kokkos::deep_copy(y1_d, y1_h);
    Kokkos::deep_copy(x2_d, x2_h);

    // Run LiVect TeamVectorRange
    Kokkos::parallel_for("lin-interp-ut-vect-ttr",
                         vect.policy(),
                         KOKKOS_LAMBDA(typename LIV::MemberType const& team_member) {
      const int i = team_member.league_rank();
      vect.setup(team_member,
                 ekat::subview(x1_d, i),
                 ekat::subview(x2_d, i));
      team_member.team_barrier();
      vect.lin_interp(team_member,
                      ekat::subview(x1_d, i),
                      ekat::subview(x2_d, i),
                      ekat::subview(y1_d, i),
                      ekat::subview(y2_d, i));
    });

    // Compare results
    Kokkos::deep_copy(y2_h, y2_d);
    auto y2_h_s = ekat::scalarize(y2_h);
    auto y1_h_s = ekat::scalarize(y1_h);
    for (int i = 0; i < ncol; ++i) {
      for (int j = 0; j < km2; ++j) {
        REQUIRE ( y2_h_s(i,j)==y1_h_s(i,2*j) );
      }
    }
  }
}

TEST_CASE("lin_interp_monotone", "lin_interp") {
  using LIV = ekat::LinInterp<Real,EKAT_TEST_PACK_SIZE>;
  using Pack = ekat::Pack<Real,EKAT_TEST_PACK_SIZE>;
  using packed_view_2d = typename LIV::template view_2d<Pack>;
  using real_pdf = std::uniform_real_distribution<Real>;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> k_dist(10,100);
  const int ncol = 10;

  real_pdf x_dist(0.0,1.0);
  real_pdf y_dist(0.0,100.0);

  // Generic lambda, to get min-max of a scalarized subview
  // Must use with rank-1 scalar views only
  auto minmax = [](const auto& v, const int sz) -> std::pair<Real,Real> {
    std::pair<Real,Real> minmax {v[0],v[0]};
    for (int i=1; i<sz; ++i) {
      minmax.first = std::min(minmax.first,v[i]);
      minmax.second = std::max(minmax.second,v[i]);
    }
    return minmax;
  };

  // increase iterations for a more-thorough testing
  for (int r = 0; r < 100; ++r) {
    const int km1 = k_dist(generator);
    const int km2 = k_dist(generator);

    // Views for testing TeamVectorRange
    LIV vect(ncol, km1, km2);
    const int km1_pack = ekat::npack<Pack>(km1);
    const int km2_pack = ekat::npack<Pack>(km2);
    packed_view_2d
      x1_d("x1", ncol, km1_pack),
      x2_d("x2", ncol, km2_pack),
      y1_d("y1", ncol, km1_pack),
      y2_d("y2", ncol, km2_pack);

    // Initialize kokkos packed inputs
    auto x1_h = Kokkos::create_mirror_view(x1_d);
    auto x2_h = Kokkos::create_mirror_view(x2_d);
    auto y1_h = Kokkos::create_mirror_view(y1_d);
    auto y2_h = Kokkos::create_mirror_view(y2_d);

    for (int i = 0; i < ncol; ++i) {
      populate_array (km1,get_col(x1_h,i).data(),generator,x_dist,true);
      populate_array (km1,get_col(y1_h,i).data(),generator,y_dist,false);

      // Generate x2 in a way that guarantees its range is contained in that of x1
      auto mm1 = minmax(get_col(x1_h,i),km1);
      auto delta = mm1.second-mm1.first;
      real_pdf x2_dist(mm1.first+delta/1000,mm1.second-delta/1000);
      populate_array (km2,get_col(x2_h,i).data(),generator,x2_dist,true);
    }
    Kokkos::deep_copy(x1_d, x1_h);
    Kokkos::deep_copy(y1_d, y1_h);
    Kokkos::deep_copy(x2_d, x2_h);

    // Run LiVect TeamVectorRange
    Kokkos::parallel_for("lin-interp-ut-vect-ttr",
                         vect.policy(),
                         KOKKOS_LAMBDA(typename LIV::MemberType const& team_member) {
      const int i = team_member.league_rank();
      vect.setup(team_member,
                 ekat::subview(x1_d, i),
                 ekat::subview(x2_d, i));
      team_member.team_barrier();
      vect.lin_interp(team_member,
                      ekat::subview(x1_d, i),
                      ekat::subview(x2_d, i),
                      ekat::subview(y1_d, i),
                      ekat::subview(y2_d, i));
    });

    // Check minmax of y2 is bounded by minmax of y1
    Kokkos::deep_copy(y2_h, y2_d);
    auto y2_h_s = ekat::scalarize(y2_h);
    auto y1_h_s = ekat::scalarize(y1_h);
    for (int i = 0; i < ncol; ++i) {
      auto mm1 = minmax(ekat::subview(y1_h_s,i),km1);
      auto mm2 = minmax(ekat::subview(y2_h_s,i),km2);
      REQUIRE ( (mm2.first>=mm1.first && mm2.second<=mm1.second) );
    }
  }
}

} // empty namespace
