#ifndef EKAT_KOKKOS_UTILS_HPP
#define EKAT_KOKKOS_UTILS_HPP

#include "ekat/kokkos/ekat_kokkos_meta.hpp"
#include "ekat/kokkos/ekat_kokkos_types.hpp"
#include "ekat/ekat_type_traits.hpp"
#include "ekat/std_meta/ekat_std_type_traits.hpp"
#include "ekat/util/ekat_arch.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/ekat_type_traits.hpp"
#include "ekat/ekat.hpp"

#include "Kokkos_Random.hpp"

#include <cassert>
#include <cstring>

// This file should not be merged with ekat/ekat_kokkos_meta.hpp.
// That file contains functionalities that *ideally* should be in kokkos
// itself, and we can foresee being in kokkos in the future. Hence, that
// file may be gone at some point (in fact, it may be gone by the time
// you read this comment). This file, instead, contains functionalities
// that are probably not generic enough to appear in kokkos any time soon
// (or ever), and are more app-specific.

namespace ekat {
namespace util {


template<typename DataTypeOut, typename DataTypeIn, typename... Props>
typename std::enable_if<GetRanks<DataTypeOut>::rank_dynamic==0,
                        typename util::Unmanaged<Kokkos::View<DataTypeOut,Props...>>>::type
reshape (Kokkos::View<DataTypeIn,Props...> view_in) {
  typename util::Unmanaged<Kokkos::View<DataTypeOut,Props...>> view_out(view_in.data());
  assert (view_in.size()==view_out.size());
  return view_out;
}

template<typename DataTypeOut, typename DataTypeIn, typename... Props>
typename std::enable_if<GetRanks<DataTypeOut>::rank_dynamic==1,
                        typename util::Unmanaged<Kokkos::View<DataTypeOut,Props...>>>::type
reshape (Kokkos::View<DataTypeIn,Props...> view_in,
         const int dim0) {
  typename util::Unmanaged<Kokkos::View<DataTypeOut,Props...>> view_out(view_in.data(),dim0);
  assert (view_in.size()==view_out.size());
  return view_out;
}

template<typename DataTypeOut, typename DataTypeIn, typename... Props>
typename std::enable_if<GetRanks<DataTypeOut>::rank_dynamic==2,
                        typename util::Unmanaged<Kokkos::View<DataTypeOut,Props...>>>::type
reshape (Kokkos::View<DataTypeIn,Props...> view_in,
         const int dim0, const int dim1) {
  typename util::Unmanaged<Kokkos::View<DataTypeOut,Props...>> view_out(view_in.data(),dim0,dim1);
  assert (view_in.size()==view_out.size());
  return view_out;
}

/*
 * ExeSpaceUtils is essentially a TeamPolicy factory. TeamPolicy objects
 * are what kokkos uses to define a thread layout (num teams, threads/team)
 * for a parallel kernel. On non-GPU archictures, we will generally have
 * thread teams of 1.
 */
template <typename ExeSpace = Kokkos::DefaultExecutionSpace>
struct ExeSpaceUtils {
  using TeamPolicy = Kokkos::TeamPolicy<ExeSpace>;

  static TeamPolicy get_default_team_policy (Int ni, Int /* nk */) {
#ifdef EKAT_MIMIC_GPU
    const int max_threads = ExeSpace::concurrency();
    const int team_size = max_threads < 7 ? max_threads : 7;
    return TeamPolicy(ni, team_size);
#else
    return TeamPolicy(ni, 1);
#endif
  }

  static TeamPolicy get_team_policy_force_team_size (Int ni, Int team_size) {
    return TeamPolicy(ni, team_size);
  }
};

/*
 * Specialization of above for Cuda execution space. Many GPU architectures can
 * support a great number of threads, so we'll need to expose additional
 * parallelism by having many threads per team.  This is due to having more
 * threads than the main kernel loop has indices.
 */
#ifdef KOKKOS_ENABLE_CUDA
template <>
struct ExeSpaceUtils<Kokkos::Cuda> {
  using TeamPolicy = Kokkos::TeamPolicy<Kokkos::Cuda>;

  static TeamPolicy get_default_team_policy (Int ni, Int nk) {
    return TeamPolicy(ni, std::min(128, 32*((nk + 31)/32)));
  }

  static TeamPolicy get_team_policy_force_team_size (Int ni, Int team_size) {
    return TeamPolicy(ni, team_size);
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
  int _team_size, _num_teams, _max_threads, _league_size;

  template <typename TeamPolicy>
  TeamUtilsCommonBase(const TeamPolicy& policy)
  {
    _max_threads = ExeSpace::concurrency();
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
  int get_num_concurrent_teams() const { return _num_teams; }

  // How many threads can run concurrently
  int get_max_concurrent_threads() const { return _max_threads; }

  // How many ws slots are there
  int get_num_ws_slots() const { return _num_teams; }

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
  template <typename TeamPolicy>
  TeamUtils(const TeamPolicy& policy, const double& = 1.0) :
    TeamUtilsCommonBase<ValueType, ExeSpace>(policy)
  { }
};

/*
 * Specialization for OpenMP execution space
 */
#ifdef KOKKOS_ENABLE_OPENMP
template <typename ValueType>
class TeamUtils<ValueType, Kokkos::OpenMP> : public TeamUtilsCommonBase<ValueType,Kokkos::OpenMP>
{
 public:
  template <typename TeamPolicy>
  TeamUtils(const TeamPolicy& policy, const double& = 1.0) :
    TeamUtilsCommonBase<ValueType,Kokkos::OpenMP>(policy)
  { }

  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  int get_workspace_idx(const MemberType& /*team_member*/) const
  { return omp_get_thread_num() / this->_team_size; }
};
#endif

/*
 * Specialization for Cuda execution space.
 */
#ifdef KOKKOS_ENABLE_CUDA
template <typename ValueType>
class TeamUtils<ValueType,Kokkos::Cuda> : public TeamUtilsCommonBase<ValueType,Kokkos::Cuda>
{
  using Device = Kokkos::Device<Kokkos::Cuda, typename Kokkos::Cuda::memory_space>;
  using flag_type = int; // this appears to be the smallest type that correctly handles atomic operations
  using view_1d = typename KokkosTypes<Device>::view_1d<flag_type>;
  using RandomGenerator = Kokkos::Random_XorShift64_Pool<Kokkos::Cuda>;
  using rnd_type = typename RandomGenerator::generator_type;

  int             _num_ws_slots;    // how many workspace slots (potentially more than the num of concurrent teams due to overprovision factor)
  bool            _need_ws_sharing; // true if there are more teams in the policy than ws slots
  view_1d         _open_ws_slots;    // indexed by ws-idx, true if in current use, else false
  RandomGenerator _rand_pool;

 public:
  template <typename TeamPolicy>
  TeamUtils(const TeamPolicy& policy, const double& overprov_factor = 1.0) :
    TeamUtilsCommonBase<ValueType,Kokkos::Cuda>(policy),
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

  // How many ws slots are there
  int get_num_ws_slots() const { return _num_ws_slots; }

  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  int get_workspace_idx(const MemberType& team_member) const
  {
    if (!_need_ws_sharing) {
      return team_member.league_rank();
    }
    else {
      int ws_idx = 0;
      Kokkos::single(Kokkos::PerTeam(team_member), [&] () {
        ws_idx = team_member.league_rank() % _num_ws_slots;
        if (!Kokkos::atomic_compare_exchange_strong(&_open_ws_slots(ws_idx), (flag_type) 0, (flag_type)1)) {
          rnd_type rand_gen = _rand_pool.get_state(team_member.league_rank());
          ws_idx = Kokkos::rand<rnd_type, int>::draw(rand_gen) % _num_ws_slots;
          while (!Kokkos::atomic_compare_exchange_strong(&_open_ws_slots(ws_idx), (flag_type) 0, (flag_type)1)) {
            ws_idx = Kokkos::rand<rnd_type, int>::draw(rand_gen) % _num_ws_slots;
          }
        }
      });

      // broadcast the idx to the team with a simple reduce
      int ws_idx_max_reduce;
      Kokkos::parallel_reduce(Kokkos::TeamThreadRange(team_member, 1), [&] (int, int& ws_idx_max) {
        ws_idx_max = ws_idx;
      }, Kokkos::Max<int>(ws_idx_max_reduce));
      team_member.team_barrier();
      return ws_idx_max_reduce;
    }
  }

  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void release_workspace_idx(const MemberType& team_member, int ws_idx) const
  {
    if (_need_ws_sharing) {
      team_member.team_barrier();
      Kokkos::single(Kokkos::PerTeam(team_member), [&] () {
        flag_type volatile* const e = &_open_ws_slots(ws_idx);
        *e = (flag_type)0;
      });
    }
  }
};
#endif

// Get a 1d subview of the i-th dimension of a 2d view
template <typename T, typename ...Parms> KOKKOS_FORCEINLINE_FUNCTION
Unmanaged<Kokkos::View<T*, Parms...> >
subview (const Kokkos::View<T**, Parms...>& v_in, const int i) {
  EKAT_KERNEL_ASSERT(v_in.data() != nullptr);
  EKAT_KERNEL_ASSERT(i < v_in.extent_int(0));
  EKAT_KERNEL_ASSERT(i >= 0);
  return util::Unmanaged<Kokkos::View<T*, Parms...> >(
    &v_in.impl_map().reference(i, 0), v_in.extent(1));
}

#ifdef KOKKOS_ENABLE_CUDA
// Replacements for namespace std functions that don't run on the GPU.
KOKKOS_INLINE_FUNCTION
size_t strlen(const char* str)
{
  EKAT_KERNEL_ASSERT(str != NULL);
  const char *char_ptr;
  for (char_ptr = str; ; ++char_ptr)  {
    if (*char_ptr == '\0') return char_ptr - str;
  }
}
KOKKOS_INLINE_FUNCTION
void strcpy(char* dst, const char* src)
{
  EKAT_KERNEL_ASSERT(dst != NULL && src != NULL);
  while(*dst++ = *src++);
}
KOKKOS_INLINE_FUNCTION
int strcmp(const char* first, const char* second)
{
  while(*first && (*first == *second))
  {
    first++;
    second++;
  }
  return *(const unsigned char*)first - *(const unsigned char*)second;
}
#else
using std::strlen;
using std::strcpy;
using std::strcmp;
#endif // KOKKOS_ENABLE_CUDA

} // namespace util
} // namespace ekat

#endif // EKAT_KOKKOS_UTILS_HPP
