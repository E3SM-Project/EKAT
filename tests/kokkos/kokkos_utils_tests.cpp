#include <catch2/catch.hpp>

#include "ekat/kokkos/ekat_subview_utils.hpp"
#include "ekat/kokkos/ekat_kokkos_utils.hpp"
#include "ekat/kokkos/ekat_kokkos_types.hpp"
#include "ekat/util/ekat_arch.hpp"
#include "ekat/ekat_pack.hpp"

#include "ekat_test_config.h"

#include <thread>

namespace {

TEST_CASE("data_type", "[kokkos_utils]") {
  using namespace ekat;

  // Check meta-util that allows to reshape a view
  Kokkos::View<double*> v1d("",100);
  auto v2d = reshape<double[2][50]>(v1d);
  REQUIRE(v2d.size()==100);

  auto v3d = reshape<double*[5][5]>(v2d,4);
  REQUIRE (v3d.size()==100);
}

TEST_CASE("team_policy", "[kokkos_utils]") {
  using namespace ekat;

  using Device = DefaultDevice;
  using ExeSpace = typename KokkosTypes<Device>::ExeSpace;

  for (int nk: {128, 122, 255, 42}) {
    const int ni = 1000;
    const auto p = ExeSpaceUtils<ExeSpace>::get_default_team_policy(ni, nk);
    REQUIRE(p.league_size() == ni);
    if (OnGpu<ExeSpace>::value) {
      if (nk == 42) {
        REQUIRE(p.team_size() == 64);
      } else {
        REQUIRE(p.team_size() == 128);
      }
    }
    else {
#if defined EKAT_MIMIC_GPU && defined KOKKOS_ENABLE_OPENMP
      REQUIRE((Kokkos::OpenMP::concurrency() == 1 || p.team_size() > 1));
#endif
    }
  }
}

TEST_CASE("team_utils_omp", "[kokkos_utils]")
{
#ifdef KOKKOS_ENABLE_OPENMP
  using namespace ekat;

  using Device = DefaultDevice;
  using ExeSpace = typename KokkosTypes<Device>::ExeSpace;
  using MemberType = typename KokkosTypes<Device>::MemberType;
  const int n = omp_get_max_threads();
  // test will not work with more than 16 threads
  if (n > 16) {
    WARN("Skipped because this test doesn't support more than 16 threads");
    return;
  }

  const int ni = n*5;
  for (int s = 1; s <= n; ++s) {
    const auto p = ExeSpaceUtils<ExeSpace>::get_team_policy_force_team_size(ni, s);
    TeamUtils<ExeSpace> tu(p);
    const int c = tu.get_num_concurrent_teams();
    typename KokkosTypes<Device>::template view_2d<int> ws_idxs("ws_idxs", ni, s);
#if 0
    const int real_ts = omp_get_max_threads() / c;
    std::cout << "thrds " << n << " teamsizeV " << s << " teamsizeR " << real_ts << " ni " << ni << " conc " << c <<  std::endl;
#endif
    int kernel_errors = 0;
    Kokkos::parallel_reduce("unittest_team_utils", p, KOKKOS_LAMBDA(MemberType team_member, int& total_errs) {
      int nerrs_local = 0;
      const int i  = team_member.league_rank();
      const int wi = tu.get_workspace_idx(team_member);

#if 0
      const int thread_num = omp_get_thread_num();
      for (int j = 0; j < n; ++j) {
        if (j == thread_num) {
          if (j == 0) {
            std::cout << "===================================" << std::endl;
          }
          std::cout << " For total_threads: " << n << " league size " << team_member.league_size() << " and team_size: " << s << ", team: " << i << ", team_rank=" << team_member.team_rank() << ", thread: " << thread_num << " , conc: " << c << ", idx: " << wi << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        team_member.team_barrier();
      }
#endif
      ws_idxs(i, team_member.team_rank()) = wi+1; // never zero
      if (wi >= c) ++nerrs_local;

      total_errs += nerrs_local;
    }, kernel_errors);
#if 0
    std::cout << "===================== DONE ==========================" << std::endl;
#endif
    REQUIRE(kernel_errors == 0);

    // post processing
    const int teams_per_idx = (ni + c - 1) / c;
    for (int i = 0; i < ni; ++i) {
      int exp_wi = 0;
      // all threads in a team should share wsidx
      for (int t = 0; t < s; ++t) {
        int curr_wi = ws_idxs(i, t);
#if 0
        std::cout << "idxs(" << i << ", " << t << ") = " << curr_wi << std::endl;
#endif
        if (t == 0) exp_wi = curr_wi;
        REQUIRE(curr_wi != 0);
        REQUIRE(curr_wi == exp_wi);
      }
    }

    // Check that each wsidx used correct number of times
    for (int ci = 1; ci <= c; ++ci) {
      for (int t = 0; t < s; ++t) {
        int cnt = 0;
        for (int i = 0; i < ni; ++i) {
          if (ws_idxs(i,t) == ci) ++cnt;
        }
        REQUIRE(cnt <= teams_per_idx);
      }
    }
  }
#endif
}

void test_utils_large_ni(const double saturation_multiplier)
{
  using namespace ekat;

  using Device = DefaultDevice;
  using ExeSpace = typename KokkosTypes<Device>::ExeSpace;
  using MemberType = typename KokkosTypes<Device>::MemberType;

  const int nk = 128;
  const double overprov_factor = 1.5;
  const auto temp_policy = ExeSpaceUtils<ExeSpace>::get_default_team_policy(1, nk);
  TeamUtils<Real,ExeSpace> tu_temp(temp_policy);
  const int num_conc = tu_temp.get_max_concurrent_threads() / temp_policy.team_size();

  int ni = num_conc*saturation_multiplier;
  if (ni == 0) ni = 1;
  const auto p = ExeSpaceUtils<ExeSpace>::get_default_team_policy(ni, nk);
  TeamUtils<Real,ExeSpace> tu(p, overprov_factor);

  REQUIRE(p.league_size() == ni);
  if (saturation_multiplier <= 1.0) {
    REQUIRE(tu.get_num_ws_slots() == ni);
  }
  else if (!OnGpu<ExeSpace>::value) {
    REQUIRE(tu.get_num_ws_slots() == num_conc);
  }
  else {
    REQUIRE(tu.get_num_ws_slots() == num_conc*overprov_factor);
  }

  int max_workspace_idx = 0;
  typename KokkosTypes<Device>::template view_1d<int> test_data("test_data", tu.get_num_ws_slots());
  Kokkos::parallel_reduce("unique_token_check", p, KOKKOS_LAMBDA(MemberType team_member, int& max_ws_idx) {
    const int wi = tu.get_workspace_idx(team_member);

    if (wi > max_ws_idx) { max_ws_idx = wi; }

    Kokkos::single(Kokkos::PerTeam(team_member), [&] () {
      int volatile* const data = &test_data(wi);
      *data += 1;
    });

    tu.release_workspace_idx(team_member, wi);
  }, Kokkos::Max<int>(max_workspace_idx));

  const auto test_data_h = Kokkos::create_mirror_view(test_data);
  Kokkos::deep_copy(test_data_h, test_data);

  int sum = 0;
  for(int i = 0; i < tu.get_num_ws_slots(); ++i) {
    sum += test_data_h(i);
  }

  REQUIRE(sum == ni);
}

TEST_CASE("team_utils_large_ni", "[kokkos_utils]")
{
  test_utils_large_ni(10);
  test_utils_large_ni(1);
  test_utils_large_ni(.5);
}

template<typename Scalar, int length, bool Serialize>
void test_parallel_reduce()
{
  using Device = ekat::DefaultDevice;
  using MemberType = typename ekat::KokkosTypes<Device>::MemberType;
  using ExeSpace = typename ekat::KokkosTypes<Device>::ExeSpace;

  // Each entry is given by data(k) = 1/(k+1)
  Scalar serial_result = Scalar();
  Kokkos::View<Scalar*, ExeSpace> data("data", length);
  const auto data_h = Kokkos::create_mirror_view(data);
  auto raw = data_h.data();
  for (int i = 0; i < length; ++i) {
    const Scalar val = Scalar(1.0/(i+1));
    serial_result += val;
    raw[i] = val;
  }
  Kokkos::deep_copy(data, data_h);

  Kokkos::View<Scalar*> results ("results", 1);
  const auto results_h = Kokkos::create_mirror_view(results);

  // parallel_for over 1 team, i.e. call parallel_reduce once
  const auto policy =
    ekat::ExeSpaceUtils<ExeSpace>::get_default_team_policy(1, length);
  Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const MemberType& team) {
    Scalar team_result = Scalar();

    const int begin = 0;
    const int end = length;
    ekat::ExeSpaceUtils<ExeSpace>::parallel_reduce<Serialize>(team, begin, end,
        [&] (const int k, Scalar& reduction_value) {
              reduction_value += data[k];
        }, team_result);

      results(0) = team_result;
    });

  Kokkos::deep_copy(results_h, results);

  // If serial computation, check bfb vs serial_result, else check to a tolerance
  if (Serialize) {
    REQUIRE(results_h(0) == serial_result);
  } else {
    REQUIRE(std::abs(results_h(0) - serial_result) <= 10*std::numeric_limits<Scalar>::epsilon());
  }
}

TEST_CASE("parallel_reduce", "[kokkos_utils]")
{
  test_parallel_reduce<Real,15,true> ();
  test_parallel_reduce<Real,15,false> ();
}


template<typename Scalar, bool Serialize, bool UseLambda, int TotalSize, int VectorSize>
void test_view_reduction(const Scalar a=Scalar(0.0), const int begin=0, const int end=TotalSize)
{
  using Device = ekat::DefaultDevice;
  using MemberType = typename ekat::KokkosTypes<Device>::MemberType;
  using ExeSpace = typename ekat::KokkosTypes<Device>::ExeSpace;
  
  using PackType = ekat::Pack<Scalar, VectorSize>;
  using ViewType = Kokkos::View<PackType*,ExeSpace>;

  const int view_length = ekat::npack<PackType>(TotalSize);

  // Each entry is given by data(k)[p] = 1/(k*Pack::n+p+1)
  Scalar serial_result = Scalar(a);
  ViewType data("data", view_length);
  const auto data_h = Kokkos::create_mirror_view(data);
  auto raw = data_h.data(); 
  for (int k = 0; k < view_length; ++k) {
    for (int p = 0; p < VectorSize; ++p) {
      const int scalar_index = k*VectorSize+p;
      if (scalar_index >= TotalSize) {
        // represents pack garbage
        raw[k][p] = ekat::ScalarTraits<Scalar>::invalid();
      } else {
        const Scalar val = 1.0/(k*VectorSize+p+1);
        raw[k][p] = val;

        if (scalar_index >= begin && scalar_index < end) {
          serial_result += val;
        }
      }
    }
  }
  Kokkos::deep_copy(data, data_h);

  Kokkos::View<Scalar*> results ("results", 1);
  const auto results_h = Kokkos::create_mirror_view(results);

  int team_size = ExeSpace::concurrency();
#ifdef KOKKOS_ENABLE_CUDA
  ExeSpace temp_space;
  auto num_sm = temp_space.impl_internal_space_instance()->m_multiProcCount;
  team_size /= (ekat::is_single_precision<Real>::value ? num_sm*64 : num_sm*32);
#endif

  // parallel_for over 1 team, i.e. call view_reduction once
  const auto policy =
    ekat::ExeSpaceUtils<ExeSpace>::get_team_policy_force_team_size(1, team_size);
  Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const MemberType& team) {
    Scalar team_result = Scalar(a);

    if (UseLambda) {
      ekat::ExeSpaceUtils<ExeSpace>::view_reduction<Serialize>(team, begin, end,
                                                               [&] (const int k) -> PackType {
                                                                 return data(k);
                                                               }, team_result);
    } else {
      ekat::ExeSpaceUtils<ExeSpace>::view_reduction<Serialize>(team, begin, end, data, team_result);
    }

    results(0) = team_result;
  });

  Kokkos::deep_copy(results_h, results);
  // If serial computation, check bfb vs serial_result, else check to a tolerance
  if (Serialize) {
    REQUIRE(results_h(0)== serial_result);
  } else {
    REQUIRE(std::abs(results_h(0) - serial_result) <= 10*std::numeric_limits<Scalar>::epsilon());
  }
}

TEST_CASE("view_reduction", "[kokkos_utils]")
{
  // VectorSize = 1

  // Sum all entries
  test_view_reduction<Real, true,true,8,1> ();
  test_view_reduction<Real,false,true,8,1> ();
  test_view_reduction<Real, true,false,8,1> ();
  test_view_reduction<Real,false,false,8,1> ();

  // Sum subset of entries, non-zero starting value, lambda data representation
  test_view_reduction<Real, true,true,8,1> (1.0/3.0,2,5);
  test_view_reduction<Real,false,true,8,1> (1.0/3.0,2,5);
  test_view_reduction<Real, true,false,8,1> (1.0/3.0,2,5);
  test_view_reduction<Real,false,false,8,1> (1.0/3.0,2,5);

  // VectorSize > 1

  // Full packs, sum all entries
  test_view_reduction<Real, true,true,8,4> ();
  test_view_reduction<Real,false,true,8,4> ();
  test_view_reduction<Real, true,false,8,4> ();
  test_view_reduction<Real,false,false,8,4> ();

  // Last pack not full, sum all entries
  test_view_reduction<Real, true,true,7,4> ();
  test_view_reduction<Real,false,true,7,4> ();
  test_view_reduction<Real, true,false,7,4> ();
  test_view_reduction<Real,false,false,7,4> ();

  // Only pack not full, sum all entries
  test_view_reduction<Real, true,true,3,4> ();
  test_view_reduction<Real,false,true,3,4> ();
  test_view_reduction<Real, true,false,3,4> ();
  test_view_reduction<Real,false,false,3,4> ();

  // Sum subset of entries, non-zero starting value
  test_view_reduction<Real, true,true,16,3> (1.0/3.0,2,11);
  test_view_reduction<Real,false,true,16,3> (1.0/3.0,2,11);
  test_view_reduction<Real, true,false,16,3> (1.0/3.0,2,11);
  test_view_reduction<Real,false,false,16,3> (1.0/3.0,2,11);

}

template<typename ViewT>
typename ViewT::HostMirror cmvc(const ViewT& v) {
  auto vh = Kokkos::create_mirror_view(v);
  Kokkos::deep_copy(vh,v);
  return vh;
}

TEST_CASE("subviews") {
  using kt = ekat::KokkosTypes<ekat::DefaultDevice>;

  const int i0 = 5;
  const int i1 = 4;
  const int i2 = 3;
  const int i3 = 2;
  const int i4 = 1;

  // Create input view
  kt::view_ND<Real,6> v6("v6",7,6,5,4,3,2);
  const int s = v6.size();
  Kokkos::parallel_for (kt::RangePolicy(0,s),
                        KOKKOS_LAMBDA(int i) {
    *(v6.data()+i) = i;
  });

  auto v5 = ekat::subview(v6,i0);
  auto v4 = ekat::subview(v6,i0,i1);
  auto v3 = ekat::subview(v6,i0,i1,i2);
  auto v2 = ekat::subview(v6,i0,i1,i2,i3);
  auto v1 = ekat::subview(v6,i0,i1,i2,i3,i4);

  SECTION ("subview_major") {

    // Subviews of v5
    auto v5_4 = ekat::subview(v5,i1);
    auto v5_3 = ekat::subview(v5,i1,i2);
    auto v5_2 = ekat::subview(v5,i1,i2,i3);
    auto v5_1 = ekat::subview(v5,i1,i2,i3,i4);

    // Subviews of v4
    auto v4_3 = ekat::subview(v4,i2);
    auto v4_2 = ekat::subview(v4,i2,i3);
    auto v4_1 = ekat::subview(v4,i2,i3,i4);

    // Subviews of v3
    auto v3_2 = ekat::subview(v3,i3);
    auto v3_1 = ekat::subview(v3,i3,i4);

    // Subviews of v2
    auto v2_1 = ekat::subview(v2,i4);

    // Compare with original view
    Kokkos::View<int> diffs("");
    Kokkos::deep_copy(diffs,0);
    Kokkos::parallel_for(kt::RangePolicy(0,1),
                         KOKKOS_LAMBDA(int) {

      int& ndiffs = diffs();
      // Check vN and vN_k against v6
      for (int m=0; m<2; ++m) {
        for (int l=0; l<3; ++l) {
          for (int k=0; k<4; ++k) {
            for (int j=0; j<5; ++j) {
              for (int i=0; i<6; ++i) {
                if (v5(i,j,k,l,m)!=v6(i0,i,j,k,l,m)) ++ndiffs;
              }
              if (v4(j,k,l,m)!=v6(i0,i1,j,k,l,m)) ++ndiffs;
              if (v5_4(j,k,l,m)!=v6(i0,i1,j,k,l,m)) ++ndiffs;
            }
            if (v3(k,l,m)!=v6(i0,i1,i2,k,l,m)) ++ndiffs;
            if (v5_3(k,l,m)!=v6(i0,i1,i2,k,l,m)) ++ndiffs;
            if (v4_3(k,l,m)!=v6(i0,i1,i2,k,l,m)) ++ndiffs;
          }
          if (v2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
          if (v5_2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
          if (v4_2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
          if (v3_2(l,m)!=v6(i0,i1,i2,i3,l,m)) ++ndiffs;
        }
        if (v1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v5_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v4_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v3_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
        if (v2_1(m)!=v6(i0,i1,i2,i3,i4,m)) ++ndiffs;
      }

      // Make sure that our diffs counting strategy works
      // by checking that two entries that should be different
      // are indeed different.
      if (v2_1(0) != v6(i0,i1,i2,i3,i4,1)) ++ndiffs;
    });
    auto diffs_h = Kokkos::create_mirror_view(diffs);
    Kokkos::deep_copy(diffs_h,diffs);
    REQUIRE (diffs_h()==1);
  }

  SECTION ("second_slowest") {
    // Subview the second slowest
    auto sv6 = ekat::subview_1(v6,i1);
    auto sv5 = ekat::subview_1(v5,i2);
    auto sv4 = ekat::subview_1(v4,i3);
    auto sv3 = ekat::subview_1(v3,i4);

    // Subview again the second slowest
    auto sv6_2 = ekat::subview_1(sv6,i2);
    auto sv5_2 = ekat::subview_1(sv5,i3);
    auto sv4_2 = ekat::subview_1(sv4,i4);

    // Compare with original view
    Kokkos::View<int> diffs("");
    Kokkos::deep_copy(diffs,0);
    Kokkos::parallel_for(kt::RangePolicy(0,1),
                         KOKKOS_LAMBDA(int) {

      int& ndiffs = diffs();
      for (int h=0; h<7; ++h)
        for (int j=0; j<5; ++j)
          for (int k=0; k<4; ++k)
            for (int l=0; l<3; ++l)
              for (int m=0; m<2; ++m) {
                if (sv6(h,j,k,l,m)!=v6(h,i1,j,k,l,m)) ++ndiffs;
              }
      for (int i=0; i<5; ++i)
        for (int k=0; k<4; ++k)
          for (int l=0; l<3; ++l)
            for (int m=0; m<2; ++m) {
              if (sv5(i,k,l,m)!=v6(i0,i,i2,k,l,m)) ++ndiffs;
            }
      for (int j=0; j<5; ++j)
        for (int l=0; l<3; ++l)
          for (int m=0; m<2; ++m) {
            if (sv4(j,l,m)!=v6(i0,i1,j,i3,l,m)) ++ndiffs;
          }
      for (int k=0; k<4; ++k)
        for (int m=0; m<2; ++m) {
          if (sv3(k,m)!=v6(i0,i1,i2,k,i4,m)) ++ndiffs;
        }


      for (int h=0; h<7; ++h)
        for (int k=0; k<4; ++k)
          for (int l=0; l<3; ++l)
            for (int m=0; m<2; ++m) {
              if (sv6_2(h,k,l,m)!=v6(h,i1,i2,k,l,m)) ++ndiffs;
            }
      for (int i=0; i<4; ++i)
        for (int l=0; l<3; ++l)
          for (int m=0; m<2; ++m) {
            if (sv5_2(i,l,m)!=v6(i0,i,i2,i3,l,m)) ++ndiffs;
          }
      for (int j=0; j<5; ++j)
        for (int m=0; m<2; ++m) {
          if (sv4_2(j,m)!=v6(i0,i1,j,i3,i4,m)) ++ndiffs;
        }

      // Make sure that our diffs counting strategy works
      // by checking that two entries that should be different
      // are indeed different.
      if (sv4_2(0,0)!=v6(i0,i1,0,i3,i4,1)) ++ndiffs;
    });
    auto diffs_h = Kokkos::create_mirror_view(diffs);
    Kokkos::deep_copy(diffs_h,diffs);
    REQUIRE (diffs_h()==1);
  }
}

} // anonymous namespace
