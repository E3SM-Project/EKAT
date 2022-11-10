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

#include <Kokkos_Random.hpp>

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

enum HostOrDevice {
  Host,
  Device
};

namespace impl {

/*
 * Computes a general parallel reduction. If Serialize=true, this reduction is computed
 * one element in order, useful for BFB testing with serial routines.
 * If Serialize=false, this function simply calls Kokkos::parallel_reduce().
 */
template <bool Serialize, typename ValueType, typename TeamMember, typename Lambda>
static KOKKOS_INLINE_FUNCTION
ValueType parallel_reduce (const TeamMember& team,
                           const int& begin,
                           const int& end,
                           const Lambda& lambda)
{
  // NOTE: initialization only necessary for the Serialize=true case.
  ValueType result = Kokkos::reduction_identity<ValueType>::sum();
  if (Serialize) {
    // Perform the reduction serially. All threads perform the same calculation,
    // (ok, since the result var is a thread-private automatic variable).
    for (int k=begin; k<end; ++k) {
      lambda(k, result);
    }
  } else {
    // Use the whole team of threads
    Kokkos::parallel_reduce(Kokkos::TeamThreadRange(team, begin, end), lambda, result);
  }
  return result;
}

/*
 * Computes a reduction over the range [begin,end) of items provided by the InputProvider
 * object, which must support calls to operator()(int).
 * We provide two overloads, one for InputProvider returning a simd-type, and one for
 * InputProvider returning a non-simd type. In both cases, the result of the reduction
 * is a non-simd type, and the [begin,end) range refers to the 'scalar' range.
 * NOTE: for simd-type usage, the simd-type must a) have a specialization of ekat::ScalarTraits
 *       that sets is_simd=true, b) support operator[](int), and c) have a compile-time size
 *       equal to the length of the simd array times the size of the individual element type.
 *       Furthermore, a specialization of Kokkos::reduction_identity<T> must be available for
 *       the simd type T.
 * NOTE: we do not provide an overload with Serialized defaulted, since this fcn is an impl
 *       detail, and, normally, should not be used by customer apps
 */
template<typename InputProvider>
using ResultType = typename std::remove_cv<
                      typename std::remove_reference<
                          decltype( std::declval<InputProvider>()(0) )
                      >::type
                   >::type;

template<typename InputProvider>
using ResultTraits = ekat::ScalarTraits<ekat::impl::ResultType<InputProvider>>;

template <bool Serialize, typename TeamMember, typename InputProvider>
static KOKKOS_INLINE_FUNCTION
auto view_reduction (const TeamMember& team,
                     const int begin, // scalar index
                     const int end, // scalar index
                     const InputProvider& input)
 -> typename std::enable_if<not ekat::impl::ResultTraits<InputProvider>::is_simd,
                            ekat::impl::ResultType<InputProvider>>::type
{
  using ValueType = typename ekat::impl::ResultType<InputProvider>;
  auto lambda = [&](const int k, ValueType& local_sum) {
    local_sum += input(k);
  };
  return impl::parallel_reduce<Serialize,ValueType>(team, begin, end, lambda);
}

template <bool Serialize, typename TeamMember, typename InputProvider>
static KOKKOS_INLINE_FUNCTION
auto view_reduction (const TeamMember& team,
                     const int begin, // scalar index
                     const int end, // scalar index
                     const InputProvider& input)
 -> typename std::enable_if<ekat::impl::ResultTraits<InputProvider>::is_simd,
                            typename ekat::impl::ResultTraits<InputProvider>::scalar_type>::type
{
  using PackType = ekat::impl::ResultType<InputProvider>;
  using ValueType = typename ekat::impl::ResultTraits<InputProvider>::scalar_type;
  constexpr int N = sizeof(PackType) / sizeof(ValueType);

  // For the serialized case, "unpack" the input provider result, and call the scalar version
  ValueType result = Kokkos::reduction_identity<ValueType>::sum();
  if (Serialize) {
    auto scalar_input = [&](const int k) {
      return input(k/N)[k%N];
    };
    result = view_reduction<true>(team,begin,end,scalar_input);
  } else {
    // For packed case, check if we need to treat first/last packs separately

    const bool has_garbage_begin = begin % N != 0;
    const bool has_garbage_end   =   end % N != 0;
    const int  pack_loop_begin   = (has_garbage_begin ? begin/N + 1 : begin/N);
    const int  pack_loop_end     = end/N;

    // If first pack has garbage, we will not include it in the below reduction,
    // so manually add the first pack (only the non-garbage part)
    if (has_garbage_begin) {
      const auto temp_input = input(pack_loop_begin-1);
      const int first_indx = begin % N;
      for (int j=first_indx; j<N; ++j) {
        result += temp_input[j];
      }
    }

    // Complete packs to be reduced. If Serialize, reduce each pack first, then
    // sum into result. Else, sum up packs, then reduce the packed sum into result.
    if (pack_loop_begin != pack_loop_end) {
      auto temp = impl::parallel_reduce<false,PackType>(
          team, pack_loop_begin, pack_loop_end,
          [&](const int k, PackType& local_packed_sum) {
            // Sum packs
            local_packed_sum += input(k);
      });
      result += ekat::reduce_sum<false>(temp);
    }

    // If last pack has garbage, we did not include it in the main reduction,
    // so manually add the last pack (only the non-garbage part)
    if (has_garbage_end) {
      const PackType temp_input = input(pack_loop_end);
      const int last_indx = end % N;
      for (int j=0; j<last_indx; ++j) {
        result += temp_input[j];
      }
    }
  }
  return result;
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

  // Note: for non-Cuda exec spaces, the template arg does nothing.
  template<HostOrDevice HD = Device>
  static TeamPolicy get_default_team_policy (Int ni, Int /* nk */) {
#ifdef EKAT_MIMIC_GPU
    const int max_threads = ExeSpace::concurrency();
    const int team_size = max_threads < 7 ? max_threads : 7;
    return TeamPolicy(ni, team_size);
#else
    return TeamPolicy(ni, 1);
#endif
  }

  template<HostOrDevice HD = Device>
  static TeamPolicy get_team_policy_force_team_size (Int ni, Int team_size) {
    return TeamPolicy(ni, team_size);
  }

  template<HostOrDevice HD = Device>
  static TeamPolicy get_thread_range_parallel_scan_team_policy (Int league_size, Int team_size_request) {
    return get_default_team_policy(league_size, team_size_request);
  }

  // NOTE: f<bool,T> and f<T,bool> are *guaranteed* to be different overloads.
  //       The latter is better when bool needs a default, the former is
  //       better when bool must be specified, but we want T to be deduced.
  // Uses ekatBFB as default for Serialize
  template <typename ValueType, typename TeamMember, typename Lambda, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  ValueType parallel_reduce (const TeamMember& team,
                             const int& begin, // pack index
                             const int& end, // pack index
                             const Lambda& lambda)
  {
    return parallel_reduce<Serialize,ValueType>(team, begin, end, lambda);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename ValueType, typename TeamMember, typename Lambda>
  static KOKKOS_INLINE_FUNCTION
  ValueType parallel_reduce (const TeamMember& team,
                             const int& begin, // pack index
                             const int& end, // pack index
                             const Lambda& lambda)
  {
    return impl::parallel_reduce<Serialize, ValueType>(team, begin, end, lambda);
  }

  // Uses ekatBFB as default for Serialize
  template <typename TeamMember, typename InputProvider, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  auto view_reduction (const TeamMember& team,
                       const int& begin, // scalar index
                       const int& end, // scalar index
                       const InputProvider& input)
   -> typename ekat::impl::ResultTraits<InputProvider>::scalar_type
  {
    return view_reduction<Serialize>(team,begin,end,input);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename TeamMember, typename InputProvider>
  static KOKKOS_INLINE_FUNCTION
  auto view_reduction (const TeamMember& team,
                       const int& begin, // scalar index
                       const int& end, // scalar index
                       const InputProvider& input)
   -> typename ekat::impl::ResultTraits<InputProvider>::scalar_type
  {
    return impl::view_reduction<Serialize>(team,begin,end,input);
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
struct ExeSpaceUtils<EkatGpuSpace> {
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
  get_policy_internal (const Int ni, const Int nk) {
    auto nk_impl = HD==Host ? 1 : nk;
    return policy_t<HD>(ni,nk_impl);
  }

  static int num_warps (const int i) {
    return (i+31)/32;
  }

  template<HostOrDevice HD = Device>
  static policy_t<HD>
  get_default_team_policy (Int ni, Int  nk ) {
    return get_policy_internal<HD>(ni, std::min(128, 32*((nk + 31)/32)));
  }

  template<HostOrDevice HD = Device>
  static policy_t<HD>
  get_team_policy_force_team_size (Int ni, Int team_size) {
    return get_policy_internal<HD>(ni, team_size);
  }

  // On GPU, the team-level ||scan in column_ops only works for team sizes that are a power of 2.
  template<HostOrDevice HD = Device>
  static policy_t<HD>
  get_thread_range_parallel_scan_team_policy (Int league_size, Int team_size_request) {
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

  // NOTE: f<bool,T> and f<T,bool> are *guaranteed* to be different overloads.
  //       The latter is better when bool needs a default, the former is
  //       better when bool must be specified, but we want T to be deduced.
  // Uses ekatBFB as default for Serialize
  template <typename ValueType, typename TeamMember, typename Lambda, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  ValueType parallel_reduce (const TeamMember& team,
                             const int& begin,
                             const int& end,
                             const Lambda& lambda)
  {
    return parallel_reduce<Serialize,ValueType>(team, begin, end, lambda);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename ValueType, typename TeamMember, typename Lambda>
  static KOKKOS_INLINE_FUNCTION
  ValueType parallel_reduce (const TeamMember& team,
                        const int& begin,
                        const int& end,
                        const Lambda& lambda)
  {
    return impl::parallel_reduce<Serialize, ValueType>(team, begin, end, lambda);
  }

  // Uses ekatBFB as default for Serialize
  template <typename TeamMember, typename InputProvider, bool Serialize = ekatBFB>
  static KOKKOS_INLINE_FUNCTION
  auto view_reduction (const TeamMember& team,
                       const int& begin,
                       const int& end,
                       const InputProvider& input)
    -> typename ekat::impl::ResultTraits<InputProvider>::scalar_type
  {
    return view_reduction<Serialize>(team,begin,end,input);
  }

  // Requires user to specify whether to serialize or not
  template <bool Serialize, typename TeamMember, typename InputProvider>
  static KOKKOS_INLINE_FUNCTION
  auto view_reduction (const TeamMember& team,
                       const int& begin,
                       const int& end,
                       const InputProvider& input)
    -> typename ekat::impl::ResultTraits<InputProvider>::scalar_type
  {
    return  impl::view_reduction<Serialize>(team,begin,end,input);
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
        if ( ! Kokkos::atomic_compare_exchange_strong(&_open_ws_slots(ws_idx), (flag_type) 0, (flag_type) 1)) {
          rnd_type rand_gen = _rand_pool.get_state(team_member.league_rank());
          ws_idx = Kokkos::rand<rnd_type, int>::draw(rand_gen) % _num_ws_slots;
          while ( ! Kokkos::atomic_compare_exchange_strong(&_open_ws_slots(ws_idx), (flag_type) 0, (flag_type) 1))
            ws_idx = Kokkos::rand<rnd_type, int>::draw(rand_gen) % _num_ws_slots;
          Kokkos::memory_fence();
        }
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
        flag_type volatile* const e = &_open_ws_slots(ws_idx);
        *e = (flag_type) 0;
        Kokkos::memory_fence();
      });
    }
  }
};
#endif

namespace impl {

#ifdef EKAT_ENABLE_GPU
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
  while((*dst++ = *src++));
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
#endif // EKAT_ENABLE_GPU

} // namespace impl

} // namespace ekat

#endif // EKAT_KOKKOS_UTILS_HPP
