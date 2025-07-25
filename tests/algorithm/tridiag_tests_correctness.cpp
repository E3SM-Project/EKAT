#include <catch2/catch.hpp>

#include "ekat_pack_kokkos.hpp"
#include "ekat_assert.hpp"

#include "tridiag_tests.hpp"

namespace ekat {
namespace test {
namespace correct {

struct Solver {
  enum Enum { thomas_team_scalar, thomas_team_pack,
              thomas_scalar, thomas_pack,
              cr_scalar, bfb,
              error };

  static std::string convert (Enum e) {
    switch (e) {
      case thomas_team_scalar: return "thomas_team_scalar";
      case thomas_team_pack: return "thomas_team_pack";
      case thomas_scalar: return "thomas_scalar";
      case thomas_pack: return "thomas_pack";
      case cr_scalar: return "cr_scalar";
      case bfb: return "bfb";
      default: EKAT_REQUIRE_MSG(false, "Not a valid solver: " << e);
    }
    return "";
  }

  static Enum convert (const std::string& s) {
    if (s == "thomas_team_scalar") return thomas_team_scalar;
    if (s == "thomas_team_pack") return thomas_team_pack;
    if (s == "thomas_scalar") return thomas_scalar;
    if (s == "thomas_pack") return thomas_pack;
    if (s == "cr_scalar") return cr_scalar;
    if (s == "bfb") return bfb;
    return error;
  }

  static Enum all[];
};

Solver::Enum Solver::all[] = { thomas_team_scalar, thomas_team_pack,
                               thomas_scalar, thomas_pack,
                               cr_scalar, bfb,
                             };

struct TestConfig {
  using TeamLayout = Kokkos::LayoutRight;

  Solver::Enum solver;
  int n_kokkos_thread, n_kokkos_vec;
};

template <typename TridiagArray>
KOKKOS_INLINE_FUNCTION
Kokkos::View<typename TridiagArray::value_type*>
get_diag (const TridiagArray& A, const int diag_idx) {
  assert(A.extent_int(2) == 1);
  return Kokkos::View<typename TridiagArray::value_type*>(
    &A.impl_map().reference(diag_idx, 0, 0),
    A.extent_int(1));
}

template <typename TridiagArray>
Kokkos::View<typename TridiagArray::value_type**, TestConfig::TeamLayout>
KOKKOS_INLINE_FUNCTION
get_diags (const TridiagArray& A, const int diag_idx) {
  return Kokkos::View<typename TridiagArray::value_type**, TestConfig::TeamLayout>(
    &A.impl_map().reference(diag_idx, 0, 0),
    A.extent_int(1), A.extent_int(2));
}

template <typename DataArray>
KOKKOS_INLINE_FUNCTION
Kokkos::View<typename DataArray::value_type*>
get_x (const DataArray& X) {
  assert(X.extent_int(1) == 1);
  return Kokkos::View<typename DataArray::value_type*>(
    &X.impl_map().reference(0, 0), X.extent_int(0));
}

template <typename Scalar>
using TridiagArray = Kokkos::View<Scalar***, TestConfig::TeamLayout>;
template <typename Scalar>
using DataArray = Kokkos::View<Scalar**, TestConfig::TeamLayout>;

template <bool same_pack_size, typename APack, typename DataPack>
struct Solve;

template <typename APack, typename DataPack>
struct Solve<true, APack, DataPack> {
  static void run (const TestConfig& tc, TridiagArray<APack>& A, DataArray<DataPack>& X,
                   const int nprob, const int nrhs) {
    using Kokkos::subview;
    using Kokkos::ALL;
    using ekat::scalarize;

    assert(nrhs > 1 || DataPack::n == 1);
    assert(nrhs > 1 || X.extent_int(2) == 1);
    assert(nprob > 1 || APack::n == 1);
    assert(nprob > 1 || A.extent_int(2) == 1);

    using TeamPolicy = Kokkos::TeamPolicy<Kokkos::DefaultExecutionSpace,
                                          Kokkos::LaunchBounds<512,2> >;
    using MT = typename TeamPolicy::member_type;
    TeamPolicy policy(1, tc.n_kokkos_thread, tc.n_kokkos_vec);

    switch (tc.solver) {
    case Solver::thomas_team_scalar:
    case Solver::thomas_team_pack: {
      const auto As = scalarize(A);
      const auto Xs = scalarize(X);
      const auto f = KOKKOS_LAMBDA (const MT& team) {
        const auto dl = get_diag(As, 0);
        const auto d  = get_diag(As, 1);
        const auto du = get_diag(As, 2);
        if (tc.solver == Solver::thomas_team_scalar)
          ekat::tridiag::thomas(team, dl, d, du, Xs);
        else
          ekat::tridiag::thomas(team, dl, d, du, X);
      };
      Kokkos::parallel_for(policy, f);
    } break;
    case Solver::thomas_scalar: {
      if (nprob == 1) {
        if (nrhs == 1) {
          const auto As = scalarize(A);
          const auto Xs = scalarize(X);
          const auto f = KOKKOS_LAMBDA (const MT& team) {
            const auto single = [&] () {
              const auto dl = get_diag(As, 0);
              const auto d  = get_diag(As, 1);
              const auto du = get_diag(As, 2);
              const auto x  = get_x(Xs);
              ekat::tridiag::thomas(dl, d, du, x);
            };
            Kokkos::single(Kokkos::PerTeam(team), single);
          };
          Kokkos::parallel_for(policy, f);
        } else {
          const auto As = scalarize(A);
          const auto Xs = scalarize(X);
          const auto f = KOKKOS_LAMBDA (const MT& team) {
            const auto single = [&] () {
              const auto dl = get_diag(As, 0);
              const auto d  = get_diag(As, 1);
              const auto du = get_diag(As, 2);
              ekat::tridiag::thomas(dl, d, du, Xs);
            };
            Kokkos::single(Kokkos::PerTeam(team), single);
          };
          Kokkos::parallel_for(policy, f);
        }
      } else {
        const auto As = scalarize(A);
        const auto Xs = scalarize(X);
        const auto f = KOKKOS_LAMBDA (const MT& team) {
          const auto single = [&] () {
            const auto dl = get_diags(As, 0);
            const auto d  = get_diags(As, 1);
            const auto du = get_diags(As, 2);
            ekat::tridiag::thomas(dl, d, du, Xs);
          };
          Kokkos::single(Kokkos::PerTeam(team), single);
        };
        Kokkos::parallel_for(policy, f);
      }
    } break;
    case Solver::thomas_pack: {
      if (nprob == 1) {
        const auto As = scalarize(A);
        const auto f = KOKKOS_LAMBDA (const MT& team) {
          const auto single = [&] () {
            const auto dl = get_diag(As, 0);
            const auto d  = get_diag(As, 1);
            const auto du = get_diag(As, 2);
            ekat::tridiag::thomas(dl, d, du, X);
          };
          Kokkos::single(Kokkos::PerTeam(team), single);
        };
        Kokkos::parallel_for(policy, f);
      } else {
        const auto f = KOKKOS_LAMBDA (const MT& team) {
          const auto single = [&] () {
            const auto dl = get_diags(A, 0);
            const auto d  = get_diags(A, 1);
            const auto du = get_diags(A, 2);
            ekat::tridiag::thomas(dl, d, du, X);
          };
          Kokkos::single(Kokkos::PerTeam(team), single);
        };
        Kokkos::parallel_for(policy, f);
      }
    } break;
    case Solver::cr_scalar: {
      if (nprob == 1) {
        if (nrhs == 1) {
          const auto As = scalarize(A);
          const auto Xs = scalarize(X);
          const auto f = KOKKOS_LAMBDA (const MT& team) {
            const auto dl = get_diag(As, 0);
            const auto d  = get_diag(As, 1);
            const auto du = get_diag(As, 2);
            const auto x  = get_x(Xs);
            ekat::tridiag::cr(team, dl, d, du, x);
          };
          Kokkos::parallel_for(policy, f);
        } else {
          const auto As = scalarize(A);
          const auto Xs = scalarize(X);
          const auto f = KOKKOS_LAMBDA (const MT& team) {
            const auto dl = get_diag(As, 0);
            const auto d  = get_diag(As, 1);
            const auto du = get_diag(As, 2);
            ekat::tridiag::cr(team, dl, d, du, Xs);
          };
         Kokkos::parallel_for(policy, f);
        }
      } else {
        const auto As = scalarize(A);
        const auto Xs = scalarize(X);
        const auto f = KOKKOS_LAMBDA (const MT& team) {
          const auto dl = get_diags(As, 0);
          const auto d  = get_diags(As, 1);
          const auto du = get_diags(As, 2);
          ekat::tridiag::cr(team, dl, d, du, Xs);
        };
        Kokkos::parallel_for(policy, f);
      }
    } break;
    case Solver::bfb: {
      const auto f = KOKKOS_LAMBDA (const MT& team) {
        const auto dl = get_diags(A, 0);
        const auto d  = get_diags(A, 1);
        const auto du = get_diags(A, 2);
        ekat::tridiag::bfb(team, dl, d, du, X);
      };
      Kokkos::parallel_for(policy, f);
    } break;
    default:
      EKAT_REQUIRE_MSG(false, "Same pack size: " << Solver::convert(tc.solver));
    }
  }
};

template <typename APack, typename DataPack>
struct Solve<false, APack, DataPack> {
  static void run (const TestConfig& tc, TridiagArray<APack>& A, DataArray<DataPack>& X,
                   const int nprob, const int nrhs) {
    using Kokkos::subview;
    using Kokkos::ALL;
    using ekat::scalarize;

    using TeamPolicy = Kokkos::TeamPolicy<Kokkos::DefaultExecutionSpace>;
    using MT = typename TeamPolicy::member_type;
    TeamPolicy policy(1, tc.n_kokkos_thread, tc.n_kokkos_vec);

    assert(nrhs > 1 || DataPack::n == 1);
    assert(nrhs > 1 || X.extent_int(2) == 1);
    assert(nprob == 1);

    switch (tc.solver) {
    case Solver::thomas_team_pack: {
      const auto As = scalarize(A);
      const auto Xs = scalarize(X);
      const auto f = KOKKOS_LAMBDA (const MT& team) {
        const auto dl = get_diag(As, 0);
        const auto d  = get_diag(As, 1);
        const auto du = get_diag(As, 2);
        if (tc.solver == Solver::thomas_team_scalar)
          ekat::tridiag::thomas(team, dl, d, du, Xs);
        else
          ekat::tridiag::thomas(team, dl, d, du, X);
      };
      Kokkos::parallel_for(policy, f);
    } break;
    case Solver::thomas_pack: {
      const auto As = scalarize(A);
      const auto f = KOKKOS_LAMBDA (const MT& team) {
        const auto single = [&] () {
          const auto dl = get_diag(As, 0);
          const auto d  = get_diag(As, 1);
          const auto du = get_diag(As, 2);
          ekat::tridiag::thomas(dl, d, du, X);
        };
        Kokkos::single(Kokkos::PerTeam(team), single);
      };
      Kokkos::parallel_for(policy, f);
    } break;
    case Solver::bfb: {
      const auto As = scalarize(A);
      const auto f = KOKKOS_LAMBDA (const MT& team) {
        const auto dl = get_diags(As, 0);
        const auto d  = get_diags(As, 1);
        const auto du = get_diags(As, 2);
        ekat::tridiag::bfb(team, dl, d, du, X);
      };
      Kokkos::parallel_for(policy, f);
    } break;
    default:
      EKAT_REQUIRE_MSG(false, "Different pack size: " << Solver::convert(tc.solver));
    }
  }
};

template <typename APack, typename DataPack>
struct Data {
  const int nprob, nrhs;
  TridiagArray<APack> A, Acopy;
  DataArray<DataPack> B, X, Y;

  Data (const int nrow, const int nprob_, const int nrhs_)
    : nprob(nprob_), nrhs(nrhs_),
      A("A", 3, nrow, ekat::npack<APack>(nprob)),
      Acopy("A", A.extent(0), A.extent(1), A.extent(2)),
      B("B", nrow, ekat::npack<DataPack>(nrhs)),
      X("X", B.extent(0), B.extent(1)),
      Y("Y", X.extent(0), X.extent(1))
  {}
};

template <typename APack, typename DataPack>
void fill (Data<APack, DataPack>& dt) {
  using Kokkos::create_mirror_view;
  using Kokkos::deep_copy;
  using Kokkos::subview;
  using Kokkos::ALL;

  const auto Am = create_mirror_view(dt.A);
  const auto Bm = create_mirror_view(dt.B);
  {
    const auto As = scalarize(Am);
    const auto dl = subview(As, 0, ALL(), ALL());
    const auto d  = subview(As, 1, ALL(), ALL());
    const auto du = subview(As, 2, ALL(), ALL());
    fill_tridiag_matrix(dl, d, du, dt.nprob, dt.nrhs /* seed */);
  }
  fill_data_matrix(scalarize(Bm), dt.nrhs);
  deep_copy(dt.A, Am);
  deep_copy(dt.B, Bm);
  deep_copy(dt.Acopy, dt.A);
  deep_copy(dt.X, dt.B);
}

template <typename APack, typename DataPack>
Real relerr (Data<APack, DataPack>& dt) {
  using Kokkos::create_mirror_view;
  using Kokkos::deep_copy;
  using Kokkos::subview;
  using Kokkos::ALL;

  const auto Acopym = create_mirror_view(dt.Acopy);
  const auto As = scalarize(Acopym);
  const auto dl = subview(As, 0, ALL(), ALL());
  const auto d  = subview(As, 1, ALL(), ALL());
  const auto du = subview(As, 2, ALL(), ALL());
  const auto Xm = create_mirror_view(dt.X);
  const auto Ym = create_mirror_view(dt.Y);
  deep_copy(Acopym, dt.Acopy);
  deep_copy(Xm, dt.X);
  matvec(dl, d, du, scalarize(Xm), scalarize(Ym), dt.nprob, dt.nrhs);
  const auto Bm = create_mirror_view(dt.B);
  deep_copy(Bm, dt.B);
  const auto re = rel_diff(scalarize(Bm), scalarize(Ym), dt.nrhs);
  return re;
}

template <typename Fn>
void run_test_configs (Fn& fn) {
  for (const auto solver : Solver::all) {
    TestConfig tc;
    tc.solver = solver;
    if (ekat::OnGpu<Kokkos::DefaultExecutionSpace>::value) {
      tc.n_kokkos_vec = 1;
      for (const int n_kokkos_thread : {128, 256, 512}) {
        tc.n_kokkos_thread = n_kokkos_thread;
        fn(tc);
      }
      tc.n_kokkos_vec = 32;
      for (const int n_kokkos_thread : {4}) {
        tc.n_kokkos_thread = n_kokkos_thread;
        fn(tc);
      }
      tc.n_kokkos_vec = 8;
      for (const int n_kokkos_thread : {16}) {
        tc.n_kokkos_thread = n_kokkos_thread;
        fn(tc);
      }
    } else {
      const int concurrency = Kokkos::DefaultExecutionSpace().concurrency();
      const int n_kokkos_thread = concurrency;
      for (const int n_kokkos_vec : {1, 2}) {
        tc.n_kokkos_thread = n_kokkos_thread;
        tc.n_kokkos_vec = n_kokkos_vec;
        fn(tc);
      }
    }
  }
}

template <int A_pack_size, int data_pack_size>
void run_property_test_on_config (const TestConfig& tc) {
  using namespace ekat::test;

  using APack = ekat::Pack<Real, A_pack_size>;
  using DataPack = ekat::Pack<Real, data_pack_size>;

  const int nrows[] = {1,2,3,4,5, 8,10,16, 32,43, 63,64,65, 111,128,129,
#ifdef NDEBUG
    2048
#endif
  };
#ifdef NDEBUG
  const int nrhs_max = 60;
  const int nrhs_inc = 11;
#else
  const int nrhs_max = 13;
  const int nrhs_inc = 4;
#endif

  for (const int nrow : nrows) {
    for (int nrhs = 1; nrhs <= nrhs_max; nrhs += nrhs_inc) {
      for (const bool A_many : {false, true}) {
        if (nrhs == 1 && A_many) continue;
        const int nprob = A_many ? nrhs : 1;

        // Skip unsupported solver-problem format combinations.
        if ((tc.solver == Solver::thomas_team_scalar ||
             tc.solver == Solver::thomas_team_pack)
            && nprob > 1)
          continue;

        // Skip combinations generated at this and higher levels that Solve::run
        // doesn't support to reduce redundancies.
        if ((nrhs  == 1 && data_pack_size > 1) ||
            (nprob == 1 && A_pack_size    > 1))
          continue;
        if ((tc.solver == Solver::thomas_team_scalar ||
             tc.solver == Solver::thomas_scalar ||
             tc.solver == Solver::cr_scalar
            ) && data_pack_size > 1)
          continue;
        if (static_cast<int>(APack::n) != static_cast<int>(DataPack::n) && nprob > 1)
          continue;

        Data<APack, DataPack> dt(nrow, nprob, nrhs);
        fill(dt);

        Solve<A_pack_size == data_pack_size, APack, DataPack>
          ::run(tc, dt.A, dt.X, nprob, nrhs);

        const auto re = relerr(dt);
        const bool pass = re <= 50*std::numeric_limits<Real>::epsilon();
        if ( ! pass) {
          std::stringstream ss;
          ss << Solver::convert(tc.solver) << " " << tc.n_kokkos_thread
             << " " << tc.n_kokkos_vec << " | " << nrow << " " << nrhs << " "
             << A_many << " | log10 rel_diff " << std::log10(re);
          std::cout << "FAIL: " << ss.str() << "\n";
        }
        REQUIRE(pass);
      }
    }
  }
}

template <int A_pack_size, int data_pack_size>
void run_property_test () {
  run_test_configs(run_property_test_on_config<A_pack_size, data_pack_size>);
}

} // namespace correct
} // namespace test
} // namespace ekat

TEST_CASE("property", "tridiag") {
  ekat::test::correct::run_property_test<1,1>();
  if (EKAT_TEST_PACK_SIZE > 1) {
    ekat::test::correct::run_property_test<1, EKAT_TEST_PACK_SIZE>();
    ekat::test::correct::run_property_test<EKAT_TEST_PACK_SIZE, EKAT_TEST_PACK_SIZE>();
  }
}
