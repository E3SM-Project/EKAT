#include <catch2/catch.hpp>

#include "ekat/util/ekat_lin_interp.hpp"
#include "ekat/util/ekat_test_utils.hpp"
#include "ekat/kokkos/ekat_subview_utils.hpp"

#include "ekat_test_config.h"
#include <random>
#include <vector>
#include <algorithm>

extern "C" {

// This will link to the fortran reference implementation
void linear_interp_c(const Real* x1, const Real* x2, const Real* y1, Real* y2, int km1, int km2, int ncol, Real minthresh);

}

namespace {

using vector_2d_t = std::vector<std::vector<Real> >;

const Real* flatten(const vector_2d_t& data)
{
  const size_t dim0 = data.size();
  const size_t dim1 = data[0].size();
  Real* result = new Real[dim0 * dim1];

  for (size_t i = 0; i < dim0; ++i) {
    std::copy(data[i].begin(), data[i].end(), result + dim1 * i);
  }

  return result;
}

template <typename Scalar>
void populate_li_input(int km1, int km2, Scalar* x1_i, Scalar* y1_i, Scalar* x2_i, std::default_random_engine* generator = nullptr)
{
  std::default_random_engine local_generator;
  if (generator == nullptr) {
    generator = &local_generator;
  }
  std::uniform_real_distribution<Real> x_dist(0.0,1.0);
  std::uniform_real_distribution<Real> y_dist(0.0,100.0);

  for (int j = 0; j < km1; ++j) {
    x1_i[j] = x_dist(*generator);
    y1_i[j] = y_dist(*generator);
  }
  for (int j = 0; j < km2; ++j) {
    x2_i[j] = x_dist(*generator);
  }

  std::sort(x1_i, x1_i + km1);
  std::sort(x2_i, x2_i + km2);
}

TEST_CASE("lin_interp_soak", "lin_interp") {

  std::default_random_engine generator;
  std::uniform_int_distribution<int> k_dist(10,100);
  const Real minthresh = 0.000001;
  const int ncol = 10;

  // increase iterations for a more-thorough soak
  for (int r = 0; r < 100; ++r) {
    const int km1 = k_dist(generator);
    const int km2 = k_dist(generator);
    vector_2d_t
      x1(ncol, std::vector<Real>(km1)),
      x2(ncol, std::vector<Real>(km2)),
      y1(ncol, std::vector<Real>(km1)),
      y2_base(ncol, std::vector<Real>(km2)),
      y2_cmp(ncol, std::vector<Real>(km2)),
      y2_f90(ncol, std::vector<Real>(km2));

    for (int i = 0; i < ncol; ++i) {
      populate_li_input(km1, km2, x1[i].data(), y1[i].data(), x2[i].data(), &generator);
    }

    // Views for testing TeamThreadRange
    using LIV = ekat::LinInterp<Real,EKAT_TEST_POSSIBLY_NO_PACK_SIZE>;
    using Pack = ekat::Pack<Real,EKAT_TEST_PACK_SIZE>;
    LIV vect(ncol, km1, km2, minthresh);
    const int km1_pack = ekat::npack<Pack>(km1);
    const int km2_pack = ekat::npack<Pack>(km2);
    typename LIV::template view_2d<Pack>
      x1kv("x1kv", ncol, km1_pack),
      x2kv("x2kv", ncol, km2_pack),
      y1kv("y1kv", ncol, km1_pack),
      y2kv("y2kv", ncol, km2_pack);

    // Views for testing ThreadVectorRange
    LIV vect1(ncol, km1, km2, minthresh);
    const int outer_dim = 2;
    const int inner_dim = ncol/outer_dim;
    typename LIV::KT::view_3d<Pack>
      x1kv3("x1kv3", outer_dim, inner_dim, km1_pack),
      x2kv3("x2kv3", outer_dim, inner_dim, km2_pack),
      y1kv3("y1kv3", outer_dim, inner_dim, km1_pack),
      y2kv3("y2kv3", outer_dim, inner_dim, km2_pack);

    // Initialize kokkos packed inputs
    {
      auto x1kvm = Kokkos::create_mirror_view(x1kv);
      auto x2kvm = Kokkos::create_mirror_view(x2kv);
      auto y1kvm = Kokkos::create_mirror_view(y1kv);

      auto x1kvm3 = Kokkos::create_mirror_view(x1kv3);
      auto x2kvm3 = Kokkos::create_mirror_view(x2kv3);
      auto y1kvm3 = Kokkos::create_mirror_view(y1kv3);

      for (int i = 0; i < ncol; ++i) {
        for (int j = 0; j < km1_pack; ++j) {
          for (int s = 0; s < Pack::n; ++s) {
            const int scalar_idx = j*Pack::n + s;
            if (scalar_idx < km1) {
              x1kvm(i, j)[s] = x1[i][scalar_idx];
              y1kvm(i, j)[s] = y1[i][scalar_idx];
              x1kvm3(i/inner_dim, i%inner_dim, j)[s] = x1[i][scalar_idx];
              y1kvm3(i/inner_dim, i%inner_dim, j)[s] = y1[i][scalar_idx];
            }
          }
        }
        for (int j = 0; j < km2_pack; ++j) {
          for (int s = 0; s < Pack::n; ++s) {
            const int scalar_idx = j*Pack::n + s;
            if (scalar_idx < km2) {
              x2kvm(i, j)[s] = x2[i][scalar_idx];
              x2kvm3(i/inner_dim, i%inner_dim, j)[s] = x2[i][scalar_idx];
            }
          }
        }
      }

      Kokkos::deep_copy(x1kv, x1kvm);
      Kokkos::deep_copy(x2kv, x2kvm);
      Kokkos::deep_copy(y1kv, y1kvm);

      Kokkos::deep_copy(x1kv3, x1kvm3);
      Kokkos::deep_copy(x2kv3, x2kvm3);
      Kokkos::deep_copy(y1kv3, y1kvm3);
    }

    std::vector<Real> x1f(ncol*km1), y1f(ncol*km1), x2f(ncol*km2), y2f(ncol*km2);

    // Initialize fortran inputs
    {
      const Real* x1flat = flatten(x1);
      const Real* x2flat = flatten(x2);
      const Real* y1flat = flatten(y1);

      ekat::transpose<ekat::TransposeDirection::c2f>(x1flat, x1f.data(), ncol, km1);
      ekat::transpose<ekat::TransposeDirection::c2f>(y1flat, y1f.data(), ncol, km1);
      ekat::transpose<ekat::TransposeDirection::c2f>(x2flat, x2f.data(), ncol, km2);

      delete[] x1flat;
      delete[] x2flat;
      delete[] y1flat;
    }

    // Run fortran and store results in y2_f90
    {
      linear_interp_c(x1f.data(), x2f.data(), y1f.data(), y2f.data(), km1, km2, ncol, minthresh);

      std::vector<Real> y2c(ncol*km2);
      ekat::transpose<ekat::TransposeDirection::f2c>(y2f.data(), y2c.data(), ncol, km2);

      for (int i = 0; i < ncol; ++i) {
        for (int j = 0; j < km2; ++j) {
          y2_f90[i][j] = y2c[i*km2 + j];
        }
      }
    }

    // Run LiVect TeamThreadRange
    {
      Kokkos::parallel_for("lin-interp-ut-vect-ttr",
                           vect.policy(),
                           KOKKOS_LAMBDA(typename LIV::MemberType const& team_member) {
        const int i = team_member.league_rank();
        vect.setup(team_member,
                   ekat::subview(x1kv, i),
                   ekat::subview(x2kv, i));
        team_member.team_barrier();
        vect.lin_interp(team_member,
                        ekat::subview(x1kv, i),
                        ekat::subview(x2kv, i),
                        ekat::subview(y1kv, i),
                        ekat::subview(y2kv, i));
      });
    }

    // Run LiVect ThreadVectorRange, doing setup from main parallel for
    {
      LIV::TeamPolicy policy(outer_dim, ekat::OnGpu<LIV::ExeSpace>::value ? inner_dim : 1, vect1.km2_pack());
      Kokkos::parallel_for("lin-interp-ut-vect-tvr", policy,
                           KOKKOS_LAMBDA(typename LIV::MemberType const& team) {
        const int i = team.league_rank();
        for (int j = 0; j < inner_dim; ++j) {
          const int col = i*inner_dim + j;
          vect1.setup(team,
                      ekat::subview(x1kv3, i, j),
                      ekat::subview(x2kv3, i, j),
                      col);
        }
        team.team_barrier();
        Kokkos::parallel_for(Kokkos::TeamThreadRange(team, inner_dim), [&] (int j) {
          const auto& tvr = Kokkos::ThreadVectorRange(team, vect1.km2_pack());
          const int col = i*inner_dim + j;
          vect1.lin_interp(team, tvr,
                           ekat::subview(x1kv3, i, j),
                           ekat::subview(x2kv3, i, j),
                           ekat::subview(y1kv3, i, j),
                           ekat::subview(y2kv3, i, j),
                           col);
        });
      });
    }

    // Compare results
    {
      auto y2kvm = Kokkos::create_mirror_view(y2kv);
      auto y2kvm3 = Kokkos::create_mirror_view(y2kv3);
      Kokkos::deep_copy(y2kvm, y2kv);
      Kokkos::deep_copy(y2kvm3, y2kv3);
      for (int i = 0; i < ncol; ++i) {
        for (int j = 0; j < km2; ++j) {
          const int k_idx = j / Pack::n;
          const int p_idx = j % Pack::n;
          ekat::catch2_req_pk_sensitive<EKAT_TEST_STRICT_FP,Pack::n>(y2_f90[i][j], y2kvm(i, k_idx)[p_idx]);
          ekat::catch2_req_pk_sensitive<EKAT_TEST_STRICT_FP,Pack::n>(y2_f90[i][j], y2kvm3(i/inner_dim, i%inner_dim, k_idx)[p_idx]);
        }
      }
    }

    // Run LiVect ThreadVectorRange, doing setup from inner TTR parallel for
    {
      LIV::TeamPolicy policy(outer_dim, ekat::OnGpu<LIV::ExeSpace>::value ? inner_dim : 1, vect1.km2_pack());
      Kokkos::parallel_for("lin-interp-ut-vect-tvr", policy,
                           KOKKOS_LAMBDA(typename LIV::MemberType const& team) {
        const int i = team.league_rank();
        Kokkos::parallel_for(Kokkos::TeamThreadRange(team, inner_dim), [&] (int j) {
          const auto& tvr = Kokkos::ThreadVectorRange(team, vect1.km2_pack());
          const int col = i*inner_dim + j;
          vect1.setup(team, tvr,
                      ekat::subview(x1kv3, i, j),
                      ekat::subview(x2kv3, i, j),
                      col);
          team.team_barrier();
          vect1.lin_interp(team, tvr,
                           ekat::subview(x1kv3, i, j),
                           ekat::subview(x2kv3, i, j),
                           ekat::subview(y1kv3, i, j),
                           ekat::subview(y2kv3, i, j),
                           col);
        });
      });
    }

    // Compare results
    {
      auto y2kvm3 = Kokkos::create_mirror_view(y2kv3);
      Kokkos::deep_copy(y2kvm3, y2kv3);
      for (int i = 0; i < ncol; ++i) {
        for (int j = 0; j < km2; ++j) {
          const int k_idx = j / Pack::n;
          const int p_idx = j % Pack::n;
          ekat::catch2_req_pk_sensitive<EKAT_TEST_STRICT_FP,Pack::n>(y2_f90[i][j], y2kvm3(i/inner_dim, i%inner_dim, k_idx)[p_idx]);
        }
      }
    }
  }
}

TEST_CASE("lin_interp_api", "lin_interp")
{
  // Test if API is flexible enough to handle various combinations
  // of const and non-const inputs

  const int ncol = 0;
  const int km1 = 0;
  const int km2 = 0;

  using LIV = ekat::LinInterp<Real,EKAT_TEST_POSSIBLY_NO_PACK_SIZE>;
  using Pack = ekat::Pack<Real,EKAT_TEST_PACK_SIZE>;
  LIV vect(ncol, km1, km2, 0);
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

} // empty namespace
