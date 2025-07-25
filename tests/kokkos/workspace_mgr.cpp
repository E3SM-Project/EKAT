#include <catch2/catch.hpp>

#include "ekat_workspace.hpp"
#include "ekat_team_policy_utils.hpp"

#include "ekat_test_config.h"

namespace unit_test {

using namespace ekat;

struct UnitWrap {

template <typename DeviceType>
struct UnitTest {

using Device     = DeviceType;
using MemberType = typename KokkosTypes<Device>::MemberType;
using TeamPolicy = typename KokkosTypes<Device>::TeamPolicy;
using ExeSpace   = typename KokkosTypes<Device>::ExeSpace;

template <typename S>
using view_1d = typename KokkosTypes<Device>::template view_1d<S>;
template <typename S>
using view_2d = typename KokkosTypes<Device>::template view_2d<S>;
template <typename S>
using view_3d = typename KokkosTypes<Device>::template view_3d<S>;

static void unittest_workspace_overprovision()
{
  using namespace ekat;

  using WSM = WorkspaceManager<double, Device>;

  const int max_threads = ExeSpace().concurrency();
  const int nk = OnGpu<ExeSpace>::value ? 128 : (max_threads < 7 ? max_threads : 7);

  const auto temp_policy = TeamPolicyFactory<ExeSpace>::get_team_policy_force_team_size(1, nk);
  TeamUtils<double,ExeSpace> tu_temp(temp_policy);
  const int num_conc = tu_temp.get_max_concurrent_threads() / temp_policy.team_size();

  constexpr double op_fact = WSM::GPU_DEFAULT_OVERPROVISION_FACTOR;
  constexpr double explicit_op_fact = op_fact * 2.0;

  const int ni_under   = (num_conc / 2) + 1;
  const int ni_conc    = num_conc;
  const int ni_between = num_conc * ( (op_fact + 1.0) / 2.0 );
  const int ni_exact   = num_conc * op_fact;
  const int ni_over    = num_conc * (explicit_op_fact + .5);

  for (const int ni_item : {ni_under, ni_conc, ni_between, ni_exact, ni_over}) {
    auto policy = TeamPolicyFactory<ExeSpace>::get_team_policy_force_team_size(ni_item, nk);
    WSM wsm(4, 4, policy);

    if (ni_item <= ni_exact) {
      REQUIRE(wsm.m_max_ws_idx == ni_item);
    }
    else if (!OnGpu<ExeSpace>::value) {
      REQUIRE(wsm.m_max_ws_idx == num_conc);
    }
    else {
      REQUIRE(wsm.m_max_ws_idx == num_conc * op_fact);
    }
  }

  for (const int ni_item : {ni_under, ni_conc, ni_between, ni_exact, ni_over}) {
    auto policy = TeamPolicyFactory<ExeSpace>::get_team_policy_force_team_size(ni_item, nk);
    WSM wsm(4, 4, policy, explicit_op_fact);

    if (ni_item <= ni_exact) {
      REQUIRE(wsm.m_max_ws_idx == ni_item);
    }
    else if (!OnGpu<ExeSpace>::value) {
      REQUIRE(wsm.m_max_ws_idx == num_conc);
    }
    else {
      REQUIRE(wsm.m_max_ws_idx == num_conc * explicit_op_fact);
    }
  }
}

static void unittest_workspace_idx_lock()
{
  using namespace ekat;

  static constexpr const int n_slots_per_team = 4;
  // For a device having ~120 SMs with 32 32-thread cores/SM, there are ~1000
  // physically placeable 4-core teams. Set up a test with league_size
  // substantially larger than that.
  const int ni = 100000;
  const int nk = 128;

  auto policy = TeamPolicyFactory<ExeSpace>::get_default_team_policy(ni, nk);
  WorkspaceManager<double, Device> wsm(nk, n_slots_per_team, policy);

  const auto f = KOKKOS_LAMBDA(const MemberType& team, int& err) {
    const auto i = team.league_rank();
    auto workspace = wsm.get_workspace(team);
    auto v = workspace.take("v");
    const auto tevr = Kokkos::TeamVectorRange(team, 0, nk);
    const auto g = [&] (const int k) { v(k) = i; };
    Kokkos::parallel_for(tevr, g);
    team.team_barrier();
    // Write race, but doesn't matter. Any err > 0 is an error.
    const auto h = [&] (const int k) { if (v(k) != i) ++err; };
    Kokkos::parallel_for(tevr, h);
    workspace.release(v);
  };
  for (int trial = 0; trial < 10; ++trial) {
    // Failures in the buggy case (missing memory_fence in
    // release_workspace_idx) are nondeterministic, so run this loop multiple
    // times to increase the strength of the test. In the buggy case on a V100,
    // I find that this test fails nearly 100% of the time.
    int err;
    Kokkos::parallel_reduce(policy, f, err);
    REQUIRE(err == 0);
  }
}

static void unittest_workspace()
{
  using namespace ekat;

  unittest_workspace_overprovision();
  unittest_workspace_idx_lock();

  static constexpr const int n_slots_per_team = 4;
  const int ni = 128;
  const int nk = 128;

  auto policy = TeamPolicyFactory<ExeSpace>::get_default_team_policy(ni, nk);

  {
    const int slot_length = 17;
    WorkspaceManager<double, Device> wsmd(slot_length, n_slots_per_team, policy);
    REQUIRE(wsmd.m_reserve == 1);
    REQUIRE(wsmd.m_size == slot_length);
  }
  {
    // Test constructing the WSM using view data
    const int slot_length = 17;
    size_t total_bytes = WorkspaceManager<double, Device>::get_total_bytes_needed(slot_length, n_slots_per_team, policy);
    size_t total_data_size = total_bytes/sizeof(double);
    view_1d<double> reserved_data("reserved_data", total_data_size);

    // Calculate next available location
    double* data_end = reserved_data.data();
    data_end += reserved_data.size();

    WorkspaceManager<double, Device> wsm(reserved_data.data(), slot_length, n_slots_per_team, policy);

    // Test that get_total_bytes_needed()/sizeof(double) returns correct n_slots
    REQUIRE(total_data_size == wsm.m_data.size());

    Kokkos::parallel_for("", policy, KOKKOS_LAMBDA(const MemberType& team) {
      auto ws = wsm.get_workspace(team);

      Unmanaged<view_1d<double> > ws1, ws2, ws3, ws4;
      ws.template take_many_contiguous_unsafe<4>(
        {"ws0", "ws1", "ws2", "ws3"},
        {&ws1, &ws2, &ws3, &ws4});

      // Assert the memory access has not exceeded the allocation
      EKAT_KERNEL_ASSERT_MSG((data_end - (ws4.data()+ws4.size())) >= 0,
                             "Error! Local view extended past allocation");

      ws.template release_many_contiguous<4>(
        {&ws1, &ws2, &ws3, &ws4});
    });
  }

  {
    // Test constructing the WSM with empty constructor
    const int slot_length = 17;
    size_t total_bytes = WorkspaceManager<double, Device>::get_total_bytes_needed(slot_length, n_slots_per_team, policy);
    size_t total_data_size = total_bytes/sizeof(double);
    view_1d<double> reserved_data("reserved_data", total_data_size);

    // Calculate next available location
    double* data_end = reserved_data.data();
    data_end += reserved_data.size();

    WorkspaceManager<double, Device> wsm1;
    WorkspaceManager<double, Device> wsm2;

    // Test both setup with and without user supplied data
    wsm1.setup(reserved_data.data(), slot_length, n_slots_per_team, policy);
    wsm2.setup(slot_length, n_slots_per_team, policy);

    // Test that get_total_bytes_needed()/sizeof(double) returns correct n_slots
    REQUIRE(total_data_size == wsm1.m_data.size());

    // Test sizes are set correctly in setup()
    REQUIRE(wsm1.m_size == slot_length);
    REQUIRE(wsm2.m_size == slot_length);
    REQUIRE(wsm1.m_reserve == 1);
    REQUIRE(wsm2.m_reserve == 1);

    // Take and release some views inside an outer iteration. This tests the need
    // for reset_internals().
    for (int n=0; n<5; ++n) {
      wsm1.reset_internals();
      wsm2.reset_internals();
      Kokkos::parallel_for("", policy, KOKKOS_LAMBDA(const MemberType& team) {
        auto ws1 = wsm1.get_workspace(team);
        auto ws2 = wsm2.get_workspace(team);

        Unmanaged<view_1d<double> > v11, v12, v13, v14,
                                    v21, v22, v23, v24;
        ws1.template take_many_contiguous_unsafe<4>(
          {"v11", "v12", "v13", "v14"},
          {&v11, &v12, &v13, &v14});
        ws2.template take_many_contiguous_unsafe<4>(
          {"v21", "v22", "v23", "v24"},
          {&v21, &v22, &v23, &v24});

        // Assert the memory access has not exceeded the allocation
        EKAT_KERNEL_ASSERT_MSG((data_end - (v14.data()+v14.size())) >= 0,
                               "Error! Local view extended past allocation");

        ws1.template release_many_contiguous<4>(
          {&v11, &v12, &v13, &v14});
        ws2.template release_many_contiguous<4>(
          {&v21, &v22, &v23, &v24});
      });
    }
  }
  {
    const int slot_length = 16;
    WorkspaceManager<char, Device> wsmc(slot_length, n_slots_per_team, policy);
    REQUIRE(wsmc.m_reserve == 8);
    REQUIRE(wsmc.m_size == slot_length);
    Kokkos::parallel_for(
      "unittest_workspace char", policy,
      KOKKOS_LAMBDA(const MemberType& team) {
        auto ws = wsmc.get_workspace(team);
        const auto t1 = ws.take("t1");
        const auto t2 = ws.take("t1");
        ws.release(t1);
        ws.release(t2);
      });
  }
  {
    const int slot_length = 16;
    WorkspaceManager<short, Device> wsms(slot_length, n_slots_per_team, policy);
    REQUIRE(wsms.m_reserve == 4);
    REQUIRE(wsms.m_size == slot_length);
  }

  // Test host-explicit WorkspaceMgr
  {
    using HostDevice = Kokkos::Device<Kokkos::DefaultHostExecutionSpace, Kokkos::HostSpace>;
    auto policy_host = TeamPolicyFactory<typename KokkosTypes<HostDevice>::ExeSpace>::get_default_team_policy(ni, nk);
    WorkspaceManager<short, HostDevice> wsmh(16, n_slots_per_team, policy_host);
    wsmh.m_data(0, 0) = 0; // check on cuda machine
  }

  // Test for various take and release functions
  {
    int nerr = 0;

    const int slot_length = 37;
    WorkspaceManager<int, Device> wsm(slot_length, n_slots_per_team, policy);

    REQUIRE(wsm.m_reserve == 2);
    REQUIRE(wsm.m_size == slot_length);

    Kokkos::parallel_reduce("unittest_workspace", policy, KOKKOS_LAMBDA(const MemberType& team, int& total_errs) {

      int nerrs_local = 0;
      auto ws = wsm.get_workspace(team);

      // Test getting workspaces of different type
      {
        const auto ws_int = ws.take("ints");
        // These nerrs_local increments are write race conditions among threads in
        // a team, but that's OK: nerrs_local doesn't have to be accurate. A 0
        // result will be a true 0 result.
        if (ws_int.extent_int(0) != slot_length) ++nerrs_local;
        ws.release(ws_int);

        const auto ws_dlb = ws.template take<double>("doubles");
        if (ws_dlb.extent(0) != 18) ++nerrs_local;
        ws.release(ws_dlb);
      }
      team.team_barrier();

      Kokkos::Array<Unmanaged<view_1d<int> >, n_slots_per_team> wssub;

      Unmanaged<view_1d<int>> wsmacro1d;
      Unmanaged<view_2d<int>> wsmacro2d;

      // Main test. Test different means of taking and release spaces.
      for (int r = 0; r < 10; ++r) {
        if (r % 5 == 0) {
          for (int w = 0; w < n_slots_per_team; ++w) {
            char buf[8] = "ws";
            buf[2] = 48 + w; // 48 is offset to integers in ascii
            wssub[w] = ws.take(buf);
          }
        }
        else if (r % 5 == 1) {
          wsmacro1d = ws.take_macro_block("wsmacro1d", n_slots_per_team);
          wsmacro2d = Unmanaged<view_2d<int>> (reinterpret_cast<int*>(wsmacro1d.data()),
                                               n_slots_per_team,slot_length);
        }
        else {
          Unmanaged<view_1d<int> > ws1, ws2, ws3, ws4;
          Kokkos::Array<Unmanaged<view_1d<int> >*, n_slots_per_team> ptrs = { {&ws1, &ws2, &ws3, &ws4} };
          Kokkos::Array<const char*, n_slots_per_team> names = { {"ws0", "ws1", "ws2", "ws3"} };
          if (r % 5 == 2) {
            // ws.take_many(names, ptrs);
            ws.take_many_refs(names,ws1,ws2,ws3,ws4);
          }
          else if (r % 5 == 3) {
            ws.take_many_contiguous_unsafe(names, ptrs);
          }
          else { // % 5 == 4
            ws.take_many_and_reset(names, ptrs);
          }

          for (int w = 0; w < n_slots_per_team; ++w) {
            wssub[w] = *ptrs[w];
          }
        }

        for (int w = 0; w < n_slots_per_team; ++w) {
          Kokkos::parallel_for(Kokkos::TeamVectorRange(team, slot_length), [&] (int i) {
            if (r % 5 == 1) {
              wsmacro2d(w,i) = i * w;
            } else {
              wssub[w](i) = i * w;
            }
          });
        }

        team.team_barrier();

        // metadata is not preserved when recasting take_macro_block data
        // so only check for other types
        if (r % 5 != 1) {
          for (int w = 0; w < n_slots_per_team; ++w) {
            // These spaces aren't free, but their metadata should be the same as it
            // was when they were initialized
            Kokkos::single(Kokkos::PerTeam(team), [&] () {
              if (wsm.get_index(wssub[w]) != w) ++nerrs_local;
              if (wsm.get_next(wssub[w]) != w+1) ++nerrs_local;
    #ifndef NDEBUG
              char buf[8] = "ws";
              buf[2] = 48 + w; // 48 is offset to integers in ascii
              if (impl::strcmp(ws.get_name(wssub[w]), buf) != 0) ++nerrs_local;
              if (ws.get_num_used() != 4) ++nerrs_local;
    #endif
              for (int i = 0; i < slot_length; ++i) {
                if (wssub[w](i) != i*w) ++nerrs_local;
              }
            });
          }
        }

        team.team_barrier();

        if (r % 5 == 0) {
          ws.reset();
        }
        else if (r % 5 == 1) {
          ws.release_macro_block(wsmacro1d,n_slots_per_team);
        }
        else if (r % 5 == 2) {
          Kokkos::Array<Unmanaged<view_1d<int> >*, n_slots_per_team> ptrs = { {&wssub[0], &wssub[1], &wssub[2], &wssub[3]} };
          ws.release_many_contiguous(ptrs);
        }
        else if (r % 5 == 3) {
          // let take_and_reset next loop do the reset
        }
        else { // % 5 == 4
          for (int w = n_slots_per_team - 1; w >= 0; --w) {
            ws.release(wssub[w]); // release individually
          }
        }

        team.team_barrier();
      }

  #ifndef EKAT_ENABLE_GPU
  #ifdef WS_EXPENSIVE_TEST
      if (true)
  #else
      if ( ExeSpace().concurrency() == 2 ) // the test below is expensive, we don't want all threads sweeps to run it
  #endif
      {
        // Test weird take/release permutations.
        for (int r = 0; r < 3; ++r) {
          int take_order[]    = {0, 1, 2, 3};
          int release_order[] = {-3, -2, -1, 0};

          do {
            for (int w = 0; w < n_slots_per_team; ++w) {
              char buf[8] = "ws";
              buf[2] = 48 + take_order[w]; // 48 is offset to integers in ascii
              wssub[take_order[w]] = ws.take(buf);
            }
            team.team_barrier();

            for (int w = 0; w < n_slots_per_team; ++w) {
              Kokkos::parallel_for(Kokkos::TeamVectorRange(team, slot_length), [&] (int i) {
                  wssub[w](i) = i * w;
                });
            }

            team.team_barrier();

            // verify stuff
            for (int w = 0; w < n_slots_per_team; ++w) {
              Kokkos::single(Kokkos::PerTeam(team), [&] () {
  #ifndef NDEBUG
                  char buf[8] = "ws";
                  buf[2] = 48 + w; // 48 is offset to integers in ascii
                  if (impl::strcmp(ws.get_name(wssub[w]), buf) != 0) ++nerrs_local;
                  if (ws.get_num_used() != 4) ++nerrs_local;
  #endif
                  for (int i = 0; i < slot_length; ++i) {
                    if (wssub[w](i) != i*w) ++nerrs_local;
                  }
                });
            }

            team.team_barrier();

            for (int w = 0; w < n_slots_per_team; ++w) {
              ws.release(wssub[release_order[w] * -1]);
            }

            team.team_barrier();

            std::next_permutation(release_order, release_order+4);

          } while (std::next_permutation(take_order, take_order+4));
        }
        ws.reset();

        // Test weird take/release permutations.
  #ifdef WS_EXPENSIVE_TEST
        {
          int actions[] = {-3, -2, -1, 1, 2, 3};
          bool exp_active[] = {false, false, false, false};

          do {
            for (int a = 0; a < 6; ++a) {
              int action = actions[a];
              if (action < 0) {
                action *= -1;
                if (exp_active[action]) {
                  ws.release(wssub[action]);
                  exp_active[action] = false;
                }
              }
              else {
                if (!exp_active[action]) {
                  char buf[8] = "ws";
                  buf[2] = 48 + action; // 48 is offset to integers in ascii
                  wssub[action] = ws.take(buf);
                  exp_active[action] = true;
                }
              }
            }

            for (int w = 0; w < n_slots_per_team; ++w) {
              if (exp_active[w]) {
                Kokkos::parallel_for(Kokkos::TeamVectorRange(team, slot_length), [&] (int i) {
                    wssub[w](i) = i * w;
                  });
              }
            }

            team.team_barrier();

            // verify stuff
            Kokkos::single(Kokkos::PerTeam(team), [&] () {
  #ifndef NDEBUG
                int exp_num_active = 0;
  #endif
                for (int w = 0; w < n_slots_per_team; ++w) {
                  if (exp_active[w]) {
  #ifndef NDEBUG
                    char buf[8] = "ws";
                    buf[2] = 48 + w; // 48 is offset to integers in ascii
                    if (impl::strcmp(ws.get_name(wssub[w]), buf) != 0) ++nerrs_local;
                    ++exp_num_active;
                    if (!ws.template is_active<int>(wssub[w])) ++nerrs_local;
  #endif
                    for (int i = 0; i < slot_length; ++i) {
                      if (wssub[w](i) != i*w) ++nerrs_local;
                    }
                  }
                }
  #ifndef NDEBUG
                if (ws.get_num_used() != exp_num_active) ++nerrs_local;
  #endif
              });

            team.team_barrier();

          } while (std::next_permutation(actions, actions + 6));
        }
  #endif
        ws.reset();
      }
  #endif

      total_errs += nerrs_local;
      team.team_barrier();
    }, nerr);

  #if 0
    wsm.report();
  #endif

    REQUIRE(nerr == 0);
  }
}

}; // struct UnitTest
}; // struct UnitWrap

} // namespace unit_test

namespace {

TEST_CASE("workspace_manager", "[utils]") {
  unit_test::UnitWrap::UnitTest<ekat::DefaultDevice>::unittest_workspace();
}

#ifdef EKAT_ENABLE_GPU
// Force host testing when the exe space is a GPU space.
TEST_CASE("workspace_manager_host", "[utils]") {
  unit_test::UnitWrap::UnitTest<ekat::HostDevice>::unittest_workspace();
}
#endif

} // anonymous namespace
