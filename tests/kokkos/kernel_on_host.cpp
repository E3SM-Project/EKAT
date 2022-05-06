#include <catch2/catch.hpp>

#include "ekat/kokkos/ekat_kokkos_utils.hpp"
#include <Kokkos_Parallel.hpp>
#include <Kokkos_DetectionIdiom.hpp>
namespace {

using namespace ekat;
using ExeSpace = typename KokkosTypes<DefaultDevice>::ExeSpace;
using MemSpace = typename KokkosTypes<DefaultDevice>::MemSpace;

enum : int {
  ValueDev = 1,
  ValueHost = -1
};

template<typename Policy>
void run (const Policy& p,
          const Kokkos::View<int**,MemSpace>& a)
{
  const int ni = a.extent(0);
  const int nk = a.extent(1);

  int value = std::is_same<typename Policy::traits::execution_space,Kokkos::Serial>::value ? ValueHost : ValueDev;
  Kokkos::parallel_for(p,KOKKOS_LAMBDA(const typename Policy::member_type& team_member) {
    const int i = team_member.league_rank();
    Kokkos::parallel_for(Kokkos::TeamThreadRange(team_member,nk),
                         [&](const int k) {
      a(i,k) = value;
    });
  });

  // We will check on host, so we need to make sure writes have finished when pages are migrated
  Kokkos::fence();
}

void check (const Kokkos::View<int**,MemSpace>& a,const int expected) {
  const int ni = a.extent(0);
  const int nk = a.extent(1);
  for (int i=0; i<ni; ++i) {
    for (int k=0; k<nk; ++k) {
      REQUIRE (a(i,k)==expected);
    }
  }
}

TEST_CASE("kernel_on_host") {
  const int ni = 2;
  const int nk = 2;

  // Note: TEST_CASE is run N times, each running a single SECTION;
  //       code outside SECTION is executed every time. So don't worry about
  //       resetting a and b after each section.
  Kokkos::View<int**,Kokkos::CudaUVMSpace> a("",ni,nk);
  Kokkos::deep_copy(a,0);

  SECTION("on_host") {
    const auto p = ExeSpaceUtils<ExeSpace>::get_default_team_policy<Host>(ni, nk);
    run(p,a);
    check(a,ValueHost);
  }

  SECTION("on_dev") {
    const auto p = ExeSpaceUtils<ExeSpace>::get_default_team_policy<Device>(ni, nk);
    run(p,a);
    check(a,ValueDev);
  }

  SECTION("default") {
    const auto p = ExeSpaceUtils<ExeSpace>::get_default_team_policy(ni, nk);
    run(p,a);
    check(a,ValueDev);
  }
}

} // anonymous namespace
