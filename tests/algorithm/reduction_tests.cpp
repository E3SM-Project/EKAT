#include <catch2/catch.hpp>

#include "ekat_reduction_utils.hpp"
#include "ekat_team_policy_utils.hpp"
#include "ekat_view_utils.hpp"

#include "ekat_test_config.h"

namespace {

template<typename Scalar, int length, bool Serialize>
void test_parallel_reduce()
{
  using Device = ekat::DefaultDevice;
  using MemberType = typename ekat::KokkosTypes<Device>::MemberType;
  using ExeSpace = typename ekat::KokkosTypes<Device>::ExeSpace;
  using ReductionUtils = ekat::ReductionUtils<ExeSpace,Serialize>;
  using PolicyFactory = ekat::PolicyFactory<ExeSpace>;

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

  // parallel_for over 1 team, i.e. call parallel_reduce once
  const auto policy = PolicyFactory::get_default_team_policy(1, length);
  Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const MemberType& team) {

    const int begin = 0;
    const int end = length;
    results(0) = ReductionUtils::template parallel_reduce<Scalar>(team, begin, end,
      [&] (const int k, Scalar& reduction_value) {
            reduction_value += data[k];
      });
  });

  const auto results_h = ekat::create_host_mirror_and_copy(results);

  // If serial computation, check bfb vs serial_result, else check to a tolerance
  if (Serialize) {
    REQUIRE(results_h(0) == serial_result);
  } else {
    REQUIRE(std::abs(results_h(0) - serial_result) <= 10*std::numeric_limits<Scalar>::epsilon());
  }
}

TEST_CASE("parallel_reduce", "[kokkos_utils]")
{
  test_parallel_reduce<Real,48,true> ();
  test_parallel_reduce<Real,48,false> ();
}

template<typename Scalar, bool Serialize, bool UseLambda, int TotalSize, int VectorSize>
void test_view_reduction(const int begin=0, const int end=TotalSize)
{
  using Device = ekat::DefaultDevice;
  using MemberType = typename ekat::KokkosTypes<Device>::MemberType;
  using ExeSpace = typename ekat::KokkosTypes<Device>::ExeSpace;
  using ReductionUtils = ekat::ReductionUtils<ExeSpace,Serialize>;
  using PolicyFactory = ekat::PolicyFactory<ExeSpace>;

  using PackType = ekat::Pack<Scalar, VectorSize>;
  using ViewType = Kokkos::View<PackType*,ExeSpace>;

  const int view_length = ekat::npack<PackType>(TotalSize);

  // Each entry is given by data(k)[p] = 1/(k*Pack::n+p+1)
  Scalar serial_result = Kokkos::reduction_identity<Scalar>::sum();
  ViewType data("data", view_length);
  const auto data_h = Kokkos::create_mirror_view(data);
  auto raw = data_h.data();
  for (int k = 0; k < view_length; ++k) {
    for (int p = 0; p < VectorSize; ++p) {
      const int scalar_index = k*VectorSize+p;
      if (scalar_index >= TotalSize) {
        // represents pack garbage
        raw[k][p] = Kokkos::Experimental::quiet_NaN_v<Scalar>;
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

  int team_size = ExeSpace().concurrency();
#ifdef EKAT_ENABLE_GPU
  ExeSpace temp_space;
  #ifdef KOKKOS_ENABLE_SYCL
  auto num_sm = temp_space.impl_internal_space_instance()->m_queue->get_device().get_info<sycl::info::device::max_compute_units>();
  #else
  auto num_sm = temp_space.cuda_device_prop().multiProcessorCount;
  #endif
  team_size /= (ekat::is_single_precision<Real>::value ? num_sm*64 : num_sm*32);
#endif

  // parallel_for over 1 team, i.e. call view_reduction once
  const auto policy = PolicyFactory::get_team_policy_force_team_size(1, team_size);
  Kokkos::parallel_for(policy, KOKKOS_LAMBDA(const MemberType& team) {
    if (UseLambda) {
      auto lambda = [&] (const int k) -> PackType {
        return data(k);
      };
      results(0) = ReductionUtils::view_reduction(team, begin, end, lambda);
    } else {
      results(0) = ReductionUtils::view_reduction(team, begin, end, data);
    }
  });

  const auto results_h = ekat::create_host_mirror_and_copy(results);
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
  test_view_reduction<Real, true, true,8,1> ();
  test_view_reduction<Real,false, true,8,1> ();
  test_view_reduction<Real, true,false,8,1> ();
  test_view_reduction<Real,false,false,8,1> ();

  // Sum subset of entries, lambda data representation
  test_view_reduction<Real, true, true,8,1> (2,5);
  test_view_reduction<Real,false, true,8,1> (2,5);
  test_view_reduction<Real, true,false,8,1> (2,5);
  test_view_reduction<Real,false,false,8,1> (2,5);

  // VectorSize > 1

  // Full packs, sum all entries
  test_view_reduction<Real, true, true,8,4> ();
  test_view_reduction<Real,false, true,8,4> ();
  test_view_reduction<Real, true,false,8,4> ();
  test_view_reduction<Real,false,false,8,4> ();

  // Last pack not full, sum all entries
  test_view_reduction<Real, true, true,7,4> ();
  test_view_reduction<Real,false, true,7,4> ();
  test_view_reduction<Real, true,false,7,4> ();
  test_view_reduction<Real,false,false,7,4> ();

  // Only pack not full, sum all entries
  test_view_reduction<Real, true, true,3,4> ();
  test_view_reduction<Real,false, true,3,4> ();
  test_view_reduction<Real, true,false,3,4> ();
  test_view_reduction<Real,false,false,3,4> ();

  // Sum subset of entries
  test_view_reduction<Real, true, true,16,4> (4,11);
  test_view_reduction<Real,false, true,16,4> (4,11);
  test_view_reduction<Real, true,false,16,4> (4,11);
  test_view_reduction<Real,false,false,16,4> (4,11);
}

} // anonymous namespace
