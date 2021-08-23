#ifndef EKAT_LIN_INTERP_HPP
#define EKAT_LIN_INTERP_HPP

#include "ekat/util/ekat_upper_bound.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/kokkos/ekat_kokkos_utils.hpp"
#include "ekat/kokkos/ekat_kokkos_types.hpp"
#include "ekat/ekat_pack.hpp"
#include "ekat/ekat_pack_kokkos.hpp"

namespace ekat {

/*
 * LinInterp is a class for doing fast linear interpolations within Kokkos
 * kernels. The user is expected to call setup for every thread team that
 * intends to do a linear interpolation. Setup is O(n log n) but it allows
 * for any number of O(n) linear interpolations using the same coordinates.
 *
 * Example: Linearly interpolate y1a, y1b, and y1c from x1 to x2
 *   Kokkos::parallel_for("setup",
                           li.m_policy,
                           KOKKOS_LAMBDA(typename LI::MemberType const& team_member) {
      const int i = team_member.league_rank();

      auto x1col = subview(x1, i);
      auto x2col = subview(x2, i);

      li.setup(team_member, x1col, x2col);

      li.lin_interp(team_member, x1col, x2col, subview(y1a, i), subview(y2a, i));
      li.lin_interp(team_member, x1col, x2col, subview(y1b, i), subview(y2b, i));
      li.lin_interp(team_member, x1col, x2col, subview(y1c, i), subview(y2c, i));
    });

  Note: testing has shown that LinInterp runs better on SKX with pack_size=1.

 */

template <typename ScalarT, int PackSize, typename DeviceT=DefaultDevice>
struct LinInterp
{
  //
  // ------- Types --------
  //

  // Expose input template args
  using Scalar = ScalarT;
  using Device = DeviceT;
  static constexpr int LI_PACKN = PackSize;

  // Other utility types
  using KT = KokkosTypes<Device>;

  template <typename S>
  using view_1d = typename KT::template view_1d<S>;
  template <typename S>
  using view_2d = typename KT::template view_2d<S>;

  using ExeSpace    = typename KT::ExeSpace;
  using MemberType  = typename KT::MemberType;
  using TeamPolicy  = typename KT::TeamPolicy;

  using Pack    = ekat::Pack<Scalar, LI_PACKN>;
  using IntPack = ekat::Pack<int, LI_PACKN>;

  //
  // ------ public API -------
  //

  LinInterp(int ncol, int km1, int km2, Scalar minthresh);

  // Simple getters
  KOKKOS_INLINE_FUNCTION
  int km1_pack() const { return m_km1_pack; }
  KOKKOS_INLINE_FUNCTION
  int km2_pack() const { return m_km2_pack; }

  const TeamPolicy& policy() const { return m_policy; }

  // Setup the index map. This must be called before lin_interp. By default, will launch a
  // TeamThreadRange kernel. By default, the column idx will be team.league_rank(); this can be
  // overridden by the col argument.
  template<typename V1, typename V2>
  KOKKOS_INLINE_FUNCTION
  void setup(
    const MemberType& team,
    const V1& x1,
    const V2& x2,
    const Int col=-1) const;

  // Same as above except uses a user-provided range boundary struct. This will likely
  // be a ThreadVectorRange.
  template<typename V1, typename V2, typename RangeBoundary>
  KOKKOS_INLINE_FUNCTION
  void setup(
    const MemberType& team,
    const RangeBoundary& range_boundary,
    const V1& x1,
    const V2& x2,
    const Int col=-1) const;

  // Linearly interpolate y(x1) onto coordinates x2. By default, will launch a
  // TeamThreadRange kernel. The x1 and x2 should match what was given to setup.
  // By default, the column idx will be team.league_rank(); this can be
  // overridden by the col argument.
  template <typename V1, typename V2, typename V3, typename V4>
  KOKKOS_INLINE_FUNCTION
  void lin_interp(
    const MemberType& team,
    const V1& x1,
    const V2& x2,
    const V3& y1,
    const V4& y2,
    const Int col=-1) const;

  // Same as above except uses a user-provided range boundary struct. This will likely
  // be a ThreadVectorRange.
  template <typename V1, typename V2, typename V3, typename V4, typename RangeBoundary>
  KOKKOS_INLINE_FUNCTION
  void lin_interp(
    const MemberType& team,
    const RangeBoundary& range_boundary,
    const V1& x1,
    const V2& x2,
    const V3& y1,
    const V4& y2,
    const Int col=-1) const;

  //
  // -------- Internal API, data ------
  //
 private:

  template <typename RangeBoundary>
  KOKKOS_INLINE_FUNCTION
  void setup_impl(
    const MemberType& team,
    const RangeBoundary& range_boundary,
    const view_1d<const Pack>& x1,
    const view_1d<const Pack>& x2,
    const Int col) const;

  template <typename RangeBoundary>
  KOKKOS_INLINE_FUNCTION
  void lin_interp_impl(
    const MemberType& team,
    const RangeBoundary& range_boundary,
    const view_1d<const Pack>& x1, const view_1d<const Pack>& x2, const view_1d<const Pack>& y1,
    const view_1d<Pack>& y2,
    const Int col) const;

  int m_km1;
  int m_km2;
  int m_km1_pack;
  int m_km2_pack;
  Scalar m_minthresh;
  TeamPolicy m_policy;
  view_2d<IntPack> m_indx_map; // [x2_idx] -> x1_idx
};

} //namespace ekat

#include "ekat_lin_interp_impl.hpp"

#endif // EKAT_LIN_INTERP_HPP
