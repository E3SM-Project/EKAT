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
#include "ekat/ekat_pack.hpp"

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

// Kokkos-compatible reduction identity for arbitrary packs
namespace Kokkos {
template<typename S, int N>
struct reduction_identity<ekat::Pack<S,N>> {
  using PackType = ekat::Pack<S,N>;

  // Provide only sum, since that's our only use case, for now
  KOKKOS_FORCEINLINE_FUNCTION
  constexpr static PackType sum() {
    return PackType (reduction_identity<S>::sum());
  }
};
} // namespace Kokkos

namespace ekat {

namespace impl {

/*
 * Computes a general parallel reduction. If Serialize=true, this reduction is computed
 * one element in order, useful for BFB testing with serial routines.
 * If Serialize=false, this function simply calls Kokkos::parallel_reduce().
 * If result contains a value, the output is result+reduction
 * For typical application, begin and end are pack indices.
 * NOTE: we do not provide an overload with Serialized defaulted, since this fcn is an impl
 *       detail, and, normally, should not be used by customer apps
 */
template <bool Serialize, typename TeamMember, typename Lambda, typename ValueType>
static KOKKOS_INLINE_FUNCTION
void parallel_reduce (const TeamMember& team,
                      const int& begin, // pack index
                      const int& end, // pack index
                      const Lambda& lambda,
                      ValueType& result)
{
  if (Serialize) {
    // We want to get C++ on GPU to match F90 on CPU. Thus, need to
    // serialize parallel reductions.

    // All threads init result.
    // NOTE: we *need* an automatic temporary, since we do not know
    //       where 'result' comes from. If result itself is an automatic
    //       variable, using it would be fine (only one vector lane would
    //       actually have a nonzero value after the single). But if
    //       result is taken from a view, then all vector lanes would
    //       see the updated value before the vector_reduce call,
    //       which will cause the final answer to be multiplied by the
    //       size of the warp.
    auto local_tmp = result;

    Kokkos::single(Kokkos::PerThread(team),[&] {
        for (int k=begin; k<end; ++k) {
          lambda(k, local_tmp);
        }
      });

#ifdef KOKKOS_ENABLE_CUDA
    // Broadcast result to all threads by doing sum of one thread's
    // non-0 value and the rest of the 0s.
    Kokkos::Impl::CudaTeamMember::vector_reduce(Kokkos::Sum<ValueType>(local_tmp));
#endif

   result = local_tmp;
  } else {
    const ValueType initial(result);
    Kokkos::parallel_reduce(Kokkos::TeamThreadRange(team, begin, end), lambda, result);
    result += initial;
  }
}

/*
 * Computes a reduction over a routine described by 'input' for entries 'scalarize(input)([begin,end))'
 * The variable 'input' should be an operator s.t. 'input(k)' returns a Pack. The computed reduction is
 * added to the value 'result'.
 * NOTE: we do not provide an overload with Serialized defaulted, since this fcn is an impl
 *       detail, and, normally, should not be used by customer apps
 */
template <bool Serialize, typename TeamMember, typename InputProvider, typename ValueType>
static KOKKOS_INLINE_FUNCTION
void view_reduction (const TeamMember& team,
                     const int begin, // scalar index
                     const int end, // scalar index
                     const InputProvider& input,
                     ValueType& result)
{
  using PackType = typename std::remove_reference<decltype(input(0))>::type;
  constexpr int vector_size = PackType::n;

  // Perform a packed reduction over scalar indices
  const bool has_garbage_begin = begin%vector_size != 0;
  const bool has_garbage_end   = end%vector_size != 0;
  const int  pack_loop_begin   = (has_garbage_begin ? begin/vector_size + 1 : begin/vector_size);
  const int  pack_loop_end     = end/vector_size;

  // If first pack has garbage, we will not include it in the below reduction,
  // so manually add the first pack (only the non-garbage part)
  if (has_garbage_begin) {
    const PackType temp_input = input(pack_loop_begin-1);
    const int first_indx = begin%vector_size;
    Kokkos::single(Kokkos::PerThread(team),[&] {
      for (int j=first_indx; j<vector_size; ++j) {
        result += temp_input[j];
      }
    });
  }

  // Complete packs to be reduced. If Serialize, reduce at pack the level, then
  // sum into result. Else, sum up packs, then sum over resulting pack into result.
  if (pack_loop_begin != pack_loop_end) {
    if (Serialize) {
      impl::parallel_reduce<Serialize>(team, pack_loop_begin, pack_loop_end,
                                       [&](const int k, ValueType& local_sum) {
          // Sum over pack entries and add to local_sum
          ekat::reduce_sum<Serialize>(input(k),local_sum);
      }, result);
    } else {
      PackType packed_result(0);
      impl::parallel_reduce<Serialize>(team, pack_loop_begin, pack_loop_end,
                                       [&](const int k, PackType& local_packed_sum) {
          // Sum packs
          local_packed_sum += input(k);
      }, packed_result);

      result += ekat::reduce_sum<Serialize>(packed_result);
    }
  }
      
  // If last pack has garbage, we did not include it in the main reduction,
  // so manually add the last pack (only the non-garbage part)
  if (has_garbage_end) {
    const PackType temp_input = input(pack_loop_end);
    // The following is morally a const var, but there are issues with
    // gnu and std=c++14. The macro ConstExceptGnu is defined in ekat_kokkos_types.hpp.
    ConstExceptGnu int last_indx = end%vector_size;
    Kokkos::single(Kokkos::PerThread(team),[&] {
      for (int j=0; j<last_indx; ++j) {
        result += temp_input[j];
      }
    });
  }
}
} //namespace impl


template<typename DataTypeOut, typename DataTypeIn, typename... Props>
typename std::enable_if<GetRanks<DataTypeOut>::rank_dynamic==0,
                        Unmanaged<Kokkos::View<DataTypeOut,Props...>>>::type
reshape (Kokkos::View<DataTypeIn,Props...> view_in) {
  typename ekat::Unmanaged<Kokkos::View<DataTypeOut,Props...>> view_out(view_in.data());
  assert (view_in.size()==view_out.size());
  return view_out;
}

template<typename DataTypeOut, typename DataTypeIn, typename... Props>
typename std::enable_if<GetRanks<DataTypeOut>::rank_dynamic==1,
                        Unmanaged<Kokkos::View<DataTypeOut,Props...>>>::type
reshape (Kokkos::View<DataTypeIn,Props...> view_in,
         const int dim0) {
  typename ekat::Unmanaged<Kokkos::View<DataTypeOut,Props...>> view_out(view_in.data(),dim0);
  assert (view_in.size()==view_out.size());
  return view_out;
}

template<typename DataTypeOut, typename DataTypeIn, typename... Props>
typename std::enable_if<GetRanks<DataTypeOut>::rank_dynamic==2,
                        Unmanaged<Kokkos::View<DataTypeOut,Props...>>>::type
reshape (Kokkos::View<DataTypeIn,Props...> view_in,
         const int dim0, const int dim1) {
  typename ekat::Unmanaged<Kokkos::View<DataTypeOut,Props...>> view_out(view_in.data(),dim0,dim1);
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

  static TeamPolicy get_thread_range_parallel_scan_team_policy (Int league_size, Int team_size_request) {
    return get_default_team_policy(league_size, team_size_request);
  }

  // NOTE: f<bool,T> and f<T,bool> are *guaranteed* to be different overloads.
  //       The latter is better when bool needs a default, the former is
  //       better when bool must be specified, but we want T to be deduced.
  // Uses ekatBFB as default for Serialize
  template <typename TeamMember, typename Lambda, typename ValueType, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  void parallel_reduce (const TeamMember& team,
                        const int& begin, // pack index
                        const int& end, // pack index
                        const Lambda& lambda,
                        ValueType& result)
  {
    parallel_reduce<Serialize>(team, begin, end, lambda, result);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename TeamMember, typename Lambda, typename ValueType>
  static KOKKOS_INLINE_FUNCTION
  void parallel_reduce (const TeamMember& team,
                        const int& begin, // pack index
                        const int& end, // pack index
                        const Lambda& lambda,
                        ValueType& result)
  {
    impl::parallel_reduce<Serialize, TeamMember, Lambda, ValueType>(team, begin, end, lambda, result);
  }

  // Uses ekatBFB as default for Serialize
  template <typename TeamMember, typename InputProvider, typename ValueType, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  void view_reduction (const TeamMember& team,
                       const int& begin, // scalar index
                       const int& end, // scalar index
                       const InputProvider& input,
                       ValueType& result)
  {
      view_reduction<Serialize>(team,begin,end,input,result);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename TeamMember, typename InputProvider, typename ValueType>
  static KOKKOS_INLINE_FUNCTION
  void view_reduction (const TeamMember& team,
                       const int& begin, // scalar index
                       const int& end, // scalar index
                       const InputProvider& input,
                       ValueType& result)
  {
      impl::view_reduction<Serialize,TeamMember,InputProvider,ValueType>(team,begin,end,input,result);
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

  static int num_warps (const int i) {
    return (i+31)/32;
  }

  static TeamPolicy get_default_team_policy (Int ni, Int nk) {
    return TeamPolicy(ni, std::min(128, 32*((nk + 31)/32)));
  }

  static TeamPolicy get_team_policy_force_team_size (Int ni, Int team_size) {
    return TeamPolicy(ni, team_size);
  }

  // On GPU, the team-level ||scan in column_ops only works for team sizes that are a power of 2.
  static TeamPolicy get_thread_range_parallel_scan_team_policy (Int league_size, Int team_size_request) {
    auto prev_pow_2 = [](const int i) -> int {
      // Multiply by 2 until pp2>i, then divide by 2 once.
      int pp2 = 1;
      while (pp2<=i) pp2 *= 2;
      return pp2/2;
    };

    const int pp2 = prev_pow_2(team_size_request);
    const int team_size = 32*num_warps(pp2);
    return TeamPolicy(league_size, std::min(128, team_size));
  }

  // NOTE: f<bool,T> and f<T,bool> are *guaranteed* to be different overloads.
  //       The latter is better when bool needs a default, the former is
  //       better when bool must be specified, but we want T to be deduced.
  // Uses ekatBFB as default for Serialize
  template <typename TeamMember, typename Lambda, typename ValueType, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  void parallel_reduce (const TeamMember& team,
                        const int& begin,
                        const int& end,
                        const Lambda& lambda,
                        ValueType& result)
  {
    parallel_reduce<Serialize>(team, begin, end, lambda, result);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename TeamMember, typename Lambda, typename ValueType>
  static KOKKOS_INLINE_FUNCTION
  void parallel_reduce (const TeamMember& team,
                        const int& begin,
                        const int& end,
                        const Lambda& lambda,
                        ValueType& result)
  {
    impl::parallel_reduce<Serialize, TeamMember, Lambda, ValueType>(team, begin, end, lambda, result);
  }

  // Uses ekatBFB as default for Serialize
  template <typename TeamMember, typename InputProvider, typename ValueType, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  void view_reduction (const TeamMember& team,
                       const int& begin,
                       const int& end,
                       const InputProvider& input,
                       ValueType& result)
  {
      view_reduction<Serialize>(team,begin,end,input,result);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename TeamMember, typename InputProvider, typename ValueType>
  static KOKKOS_INLINE_FUNCTION
  void view_reduction (const TeamMember& team,
                       const int& begin,
                       const int& end,
                       const InputProvider& input,
                       ValueType& result)
  {
      impl::view_reduction<Serialize,TeamMember,InputProvider,ValueType>(team,begin,end,input,result);
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

namespace impl {

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

} // namespace impl

} // namespace ekat

#endif // EKAT_KOKKOS_UTILS_HPP
