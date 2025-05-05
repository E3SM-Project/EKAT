#ifndef EKAT_TEAM_POLICY_UTILS_HPP
#define EKAT_TEAM_POLICY_UTILS_HPP

#include "ekat_kokkos_types.hpp"
#include "ekat_type_traits.hpp"
#include "ekat_kernel_assert.hpp"
#include "ekat_assert.hpp"

#include <Kokkos_Random.hpp>

namespace ekat {

/*
 * A class to create team policies
 */
template <typename ExeSpace>
struct PolicyFactory {
  using TeamPolicy = Kokkos::TeamPolicy<ExeSpace>;

  // Note: for non-Cuda exec spaces, the template arg does nothing.
  template<HostOrDevice HD = Device>
  static TeamPolicy get_default_team_policy (int ni, int /* nk */) {
#ifdef EKAT_MIMIC_GPU
    const int max_threads = ExeSpace().concurrency();
    const int team_size = max_threads < 7 ? max_threads : 7;
    return TeamPolicy(ni, team_size);
#else
    return TeamPolicy(ni, 1);
#endif
  }

  template<HostOrDevice HD = Device>
  static TeamPolicy get_team_policy_force_team_size (int ni, int team_size) {
    return TeamPolicy(ni, team_size);
  }

  template<HostOrDevice HD = Device>
  static TeamPolicy get_thread_range_parallel_scan_team_policy (int league_size, int team_size_request) {
    return get_default_team_policy(league_size, team_size_request);
  }
};

/*
 * Specialization of above for Cuda execution space. Many GPU architectures can
 * support a great number of threads, so we'll need to expose additional
 * parallelism by having many threads per team.  This is due to having more
 * threads than the main kernel loop has indices.
 */
#ifdef EKAT_ENABLE_GPU
template <>
struct PolicyFactory<EkatGpuSpace> {
  using TeamPolicy = Kokkos::TeamPolicy<EkatGpuSpace>;
  using HostTeamPolicy = Kokkos::TeamPolicy<Kokkos::Serial>;

  // Enable policy on Host only if UVM is enabled.
  template<HostOrDevice HD>
  struct PolicyOnHostHelper {
    static constexpr bool UseUVM =
#ifdef KOKKOS_ENABLE_CUDA
      std::is_same<Kokkos::Cuda::memory_space,Kokkos::CudaUVMSpace>::value
#else
      false
#endif
      ;
    static_assert (HD==Device  || UseUVM, "Error! Cannot get a policy on Host unless Cuda UVM is enabled in Kokkos.");
    using type = typename std::conditional<HD==Host,HostTeamPolicy,TeamPolicy>::type;
  };

  template<HostOrDevice HD>
  using policy_t = typename PolicyOnHostHelper<HD>::type;

  template<HostOrDevice HD = Device>
  static policy_t<HD>
  get_policy_internal (const int ni, const int nk) {
    auto nk_impl = HD==Host ? 1 : nk;
    return policy_t<HD>(ni,nk_impl);
  }

  static int num_warps (const int i) {
    return (i+31)/32;
  }

  template<HostOrDevice HD = Device>
  static policy_t<HD>
  get_default_team_policy (int ni, int  nk ) {
    return get_policy_internal<HD>(ni, std::min(128, 32*((nk + 31)/32)));
  }

  template<HostOrDevice HD = Device>
  static policy_t<HD>
  get_team_policy_force_team_size (int ni, int team_size) {
    return get_policy_internal<HD>(ni, team_size);
  }

  // On GPU, the team-level ||scan in column_ops only works for team sizes that are a power of 2.
  template<HostOrDevice HD = Device>
  static policy_t<HD>
  get_thread_range_parallel_scan_team_policy (int league_size, int team_size_request) {
    auto prev_pow_2 = [](const int i) -> int {
      // Multiply by 2 until pp2>i, then divide by 2 once.
      int pp2 = 1;
      while (pp2<=i) pp2 *= 2;
      return pp2/2;
    };

    const int pp2 = prev_pow_2(team_size_request);
    const int team_size = 32*num_warps(pp2);
    return get_policy_internal<HD>(league_size, std::min(128, team_size));
  }
};
#endif

/*
 * TeamUtils contains utilities for getting concurrency info for thread teams.
 * You cannot use it directly (protected c-tor). You must use TeamUtils.
 * NOTE: the ValueType template arg is the type of scalars of the views that
 *       you intend to do parallel work on. We need to know that, because if
 *       the ValueType=float, we have 2x concurrency available on GPU
 *       compared to ValueType=double.
 */

template <typename ValueType, typename ExeSpace = Kokkos::DefaultExecutionSpace>
class TeamUtilsCommonBase
{
protected:
  int _team_size=0, _num_teams, _max_threads, _league_size;

  TeamUtilsCommonBase() = default;

  template <typename TeamPolicy>
  TeamUtilsCommonBase(const TeamPolicy& policy)
  {
    _max_threads = ExeSpace().concurrency();
    if (!is_single_precision<ValueType>::value && OnGpu<ExeSpace>::value) {
      _max_threads /= 2;
    }
    const int team_size = policy.team_size();
    _num_teams = _max_threads / team_size;
    _team_size = _max_threads / _num_teams;
    _league_size = policy.league_size();

    // We will never run more teams than the policy needs
    _num_teams = _num_teams > _league_size ? _league_size : _num_teams;

    EKAT_ASSERT_MSG(_num_teams > 0, "Should always be able to run at least 1 team."
                                    "\n max_thrds   = " + std::to_string(_max_threads) +
                                    "\n team_size   = " + std::to_string(team_size) +
                                    "\n league_size = " + std::to_string(_league_size) + "\n");
  }

public:

  // How many thread teams can run concurrently
  int get_num_concurrent_teams() const
  {
    EKAT_ASSERT_MSG (_team_size>0, "Error! TeamUtils not yet inited.\n");
    return _num_teams;
  }

  // How many threads can run concurrently
  int get_max_concurrent_threads() const
  {
    EKAT_ASSERT_MSG (_team_size>0, "Error! TeamUtils not yet inited.\n");
    return _max_threads;
  }

  // How many ws slots are there
  int get_num_ws_slots() const
  {
    EKAT_ASSERT_MSG (_team_size>0, "Error! TeamUtils not yet inited.\n");
    return _num_teams;
  }

  /*
   * Of the C concurrently running teams, which "slot" is open
   * for the given team.
   */
  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  int get_workspace_idx(const MemberType& /*team_member*/) const
  { return 0; }

  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void release_workspace_idx(const MemberType& /*team_member*/, int /*ws_idx*/) const
  { }
};

template <typename ValueType, typename ExeSpace = Kokkos::DefaultExecutionSpace>
class TeamUtils : public TeamUtilsCommonBase<ValueType, ExeSpace>
{
 public:
  TeamUtils() = default;

  template <typename TeamPolicy>
  TeamUtils(const TeamPolicy& policy, const double& = 1.0) :
    TeamUtilsCommonBase<ValueType, ExeSpace>(policy)
  { }

  TeamUtils& operator= (const TeamUtils& src) = default;
};

/*
 * Specialization for OpenMP execution space
 */
#ifdef KOKKOS_ENABLE_OPENMP
template <typename ValueType>
class TeamUtils<ValueType, Kokkos::OpenMP> : public TeamUtilsCommonBase<ValueType,Kokkos::OpenMP>
{
 public:
  TeamUtils() = default;

  template <typename TeamPolicy>
  TeamUtils(const TeamPolicy& policy, const double& = 1.0) :
    TeamUtilsCommonBase<ValueType,Kokkos::OpenMP>(policy)
  { }

  TeamUtils& operator= (const TeamUtils& src) = default;

  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  int get_workspace_idx(const MemberType& /*team_member*/) const
  {
    EKAT_KERNEL_ASSERT_MSG (this->_team_size>0, "Error! TeamUtils not yet inited.\n");
    return omp_get_thread_num() / this->_team_size;
  }
};
#endif

/*
 * Specialization for CUDA, HIP and SYCL execution space.
 */
#ifdef EKAT_ENABLE_GPU
template <typename ValueType>
class TeamUtils<ValueType,EkatGpuSpace> : public TeamUtilsCommonBase<ValueType,EkatGpuSpace>
{
  using Device = Kokkos::Device<EkatGpuSpace, typename EkatGpuSpace::memory_space>;
  using flag_type = int; // this appears to be the smallest type that correctly handles atomic operations
  using view_1d = typename KokkosTypes<Device>::view_1d<flag_type>;
  using RandomGenerator = Kokkos::Random_XorShift64_Pool<EkatGpuSpace>;
  using rnd_type = typename RandomGenerator::generator_type;

  int             _num_ws_slots;    // how many workspace slots (potentially more than the num of concurrent teams due to overprovision factor)
  bool            _need_ws_sharing; // true if there are more teams in the policy than ws slots
  view_1d         _open_ws_slots;    // indexed by ws-idx, true if in current use, else false
  RandomGenerator _rand_pool;

 public:
  TeamUtils() = default;

  template <typename TeamPolicy>
  TeamUtils(const TeamPolicy& policy, const double& overprov_factor = 1.0) :
    TeamUtilsCommonBase<ValueType,EkatGpuSpace>(policy),
    _num_ws_slots(this->_league_size > this->_num_teams
                  ? (overprov_factor * this->_num_teams > this->_league_size ? this->_league_size : overprov_factor * this->_num_teams)
                  : this->_num_teams),
    _need_ws_sharing(this->_league_size > _num_ws_slots),
    _open_ws_slots("open_ws_slots", _need_ws_sharing ? _num_ws_slots : 0),
    _rand_pool()
  {
    if (_need_ws_sharing) {
      _rand_pool = RandomGenerator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
  }

  TeamUtils& operator= (const TeamUtils& src) = default;

  // How many ws slots are there
  int get_num_ws_slots() const
  {
    EKAT_ASSERT_MSG (this->_team_size>0, "Error! TeamUtils not yet inited.\n");
    return _num_ws_slots;
  }

  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  int get_workspace_idx(const MemberType& team_member) const
  {
    EKAT_KERNEL_ASSERT_MSG (this->_num_teams > 0, "Error! TeamUtils not yet inited.\n");

    if ( ! _need_ws_sharing) {
      return team_member.league_rank();
    } else {
      int ws_idx_broadcast;
      Kokkos::single(Kokkos::PerTeam(team_member), [&] (int& ws_idx) {
        ws_idx = team_member.league_rank() % _num_ws_slots;
        if ((flag_type) 0 != Kokkos::atomic_compare_exchange(&_open_ws_slots(ws_idx), (flag_type) 0, (flag_type) 1)) {
          rnd_type rand_gen = _rand_pool.get_state(team_member.league_rank());
          ws_idx = Kokkos::rand<rnd_type, int>::draw(rand_gen) % _num_ws_slots;
          while ((flag_type) 0 != Kokkos::atomic_compare_exchange(&_open_ws_slots(ws_idx), (flag_type) 0, (flag_type) 1))
            ws_idx = Kokkos::rand<rnd_type, int>::draw(rand_gen) % _num_ws_slots;
        }
        // The following memory fence is not strictly needed if the application
        // code uses fences where it should for reads and writes to global
        // resources. However, it's simplest to call it here so the app doesn't
        // have to worry about it. The workspace debug code is an example of a
        // caller that needs this. Without it, the test
        // 'unittest_workspace_idx_lock' fails with an assertion in the
        // debug-enabled workspace code due to out-of-sequence reads.
        Kokkos::memory_fence();
      }, ws_idx_broadcast);
      return ws_idx_broadcast;
    }
  }

  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void release_workspace_idx(const MemberType& team_member, int ws_idx) const
  {
    if (_need_ws_sharing) {
      team_member.team_barrier();
      Kokkos::single(Kokkos::PerTeam(team_member), [&] () {
        // The 'volatile' declaration in atomic_compare_exchange and in
        // the following code means that all threads on the GPU will see the
        // correct value of the lock when they access it. But the values in
        // writes a thread makes to global memory are not necessarily seen by
        // other threads as occurring in the same sequence. Without a memory
        // fence, it's possible for thread B to release the lock, A to acquire
        // the lock, A to start using the resource protected by the lock, and
        // then get a delayed read of a write to that resource B made before B
        // released the lock. With this memory fence, any writes B makes to the
        // resource will be read by A as occurring before the write B makes to
        // the lock, thus assuring the global resource is truly free from A's
        // perspective when it acquires the lock.
        Kokkos::memory_fence();                          
        flag_type volatile* const e = &_open_ws_slots(ws_idx);
        *e = (flag_type) 0;
      });
    }
  }
};
#endif

} // namespace ekat

#endif // EKAT_TEAM_POLICY_UTILS_HPP
