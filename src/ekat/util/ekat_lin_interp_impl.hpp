#ifndef EKAT_LIN_INTERP_HPP
#include "ekat_lin_interp.hpp"
#endif

namespace ekat {

// Never include this header directly, only ekat_lin_interp.hpp should include it

template <typename ScalarT, int PackSize, typename DeviceT>
LinInterp<ScalarT, PackSize, DeviceT>::LinInterp(int ncol, int km1, int km2, Scalar minthresh) :
  m_km1(km1),
  m_km2(km2),
  m_km1_pack(ekat::npack<Pack>(km1)),
  m_km2_pack(ekat::npack<Pack>(km2)),
  m_minthresh(minthresh),
  m_policy(ExeSpaceUtils<ExeSpace>::get_default_team_policy(ncol, m_km2_pack)),
  m_indx_map("m_indx_map", ncol, ekat::npack<IntPack>(km2))
{}

template <typename ScalarT, int PackSize, typename DeviceT>
template<typename V1, typename V2>
KOKKOS_INLINE_FUNCTION
void LinInterp<ScalarT, PackSize, DeviceT>::setup(
  const MemberType& team,
  const V1& x1,
  const V2& x2,
  const Int col) const
{
  setup_impl(team, Kokkos::TeamThreadRange(team, m_km2_pack),
             ekat::repack<Pack::n>(x1), ekat::repack<Pack::n>(x2), col);
}

template <typename ScalarT, int PackSize, typename DeviceT>
template<typename V1, typename V2, typename RangeBoundary>
KOKKOS_INLINE_FUNCTION
void LinInterp<ScalarT, PackSize, DeviceT>::setup(
  const MemberType& team,
  const RangeBoundary& range_boundary,
  const V1& x1,
  const V2& x2,
  const Int col) const
{
  setup_impl(team, range_boundary,
             ekat::repack<Pack::n>(x1), ekat::repack<Pack::n>(x2), col);
}

template <typename ScalarT, int PackSize, typename DeviceT>
template <typename V1, typename V2, typename V3, typename V4>
KOKKOS_INLINE_FUNCTION
void LinInterp<ScalarT, PackSize, DeviceT>::lin_interp(
  const MemberType& team,
  const V1& x1,
  const V2& x2,
  const V3& y1,
  const V4& y2,
  const Int col) const
{
  lin_interp_impl(team,
                  Kokkos::TeamThreadRange(team, m_km2_pack),
                  ekat::repack<Pack::n>(x1),
                  ekat::repack<Pack::n>(x2),
                  ekat::repack<Pack::n>(y1),
                  ekat::repack<Pack::n>(y2),
                  col);
}

template <typename ScalarT, int PackSize, typename DeviceT>
template <typename V1, typename V2, typename V3, typename V4, typename RangeBoundary>
KOKKOS_INLINE_FUNCTION
void LinInterp<ScalarT, PackSize, DeviceT>::lin_interp(
  const MemberType& team,
  const RangeBoundary& range_boundary,
  const V1& x1,
  const V2& x2,
  const V3& y1,
  const V4& y2,
  const Int col) const
{
  lin_interp_impl(team,
                  range_boundary,
                  ekat::repack<Pack::n>(x1),
                  ekat::repack<Pack::n>(x2),
                  ekat::repack<Pack::n>(y1),
                  ekat::repack<Pack::n>(y2),
                  col);
}

template <typename ScalarT, int PackSize, typename DeviceT>
template <typename RangeBoundary>
KOKKOS_INLINE_FUNCTION
void LinInterp<ScalarT, PackSize, DeviceT>::lin_interp_impl(
  const MemberType& team,
  const RangeBoundary& range_boundary,
  const view_1d<const Pack>& x1, const view_1d<const Pack>& x2, const view_1d<const Pack>& y1,
  const view_1d<Pack>& y2,
  const Int col) const
{
  auto x1s = ekat::scalarize(x1);
  auto y1s = ekat::scalarize(y1);

  const int i = col == -1 ? team.league_rank() : col;
  Kokkos::parallel_for(range_boundary, [&] (Int k2) {
    const auto indx_pk = m_indx_map(i, k2);
    const auto end_mask = indx_pk == m_km1 - 1;
    if (end_mask.any()) {
      const auto not_end = !end_mask;
      ekat_masked_loop(end_mask, s) {
        int k1 = indx_pk[s];
        y2(k2)[s] = y1s(k1) + (y1s(k1)-y1s(k1-1))*(x2(k2)[s]-x1s(k1))/(x1s(k1)-x1s(k1-1));
      }
      ekat_masked_loop(not_end, s) {
        int k1 = indx_pk[s];
        y2(k2)[s] = y1s(k1) + (y1s(k1+1)-y1s(k1))*(x2(k2)[s]-x1s(k1))/(x1s(k1+1)-x1s(k1));
      }
    }
    else {
      Pack x1p, x1p1, y1p, y1p1;
      ekat::index_and_shift<1>(x1s, indx_pk, x1p, x1p1);
      ekat::index_and_shift<1>(y1s, indx_pk, y1p, y1p1);
      const auto& x2p = x2(k2);

      y2(k2) = y1p + (y1p1-y1p)*(x2p-x1p)/(x1p1-x1p);
    }

    y2(k2).set(y2(k2) < m_minthresh, m_minthresh);
  });
}

template <typename ScalarT, int PackSize, typename DeviceT>
template <typename RangeBoundary>
KOKKOS_INLINE_FUNCTION
void LinInterp<ScalarT, PackSize, DeviceT>::setup_impl(
  const MemberType& team,
  const RangeBoundary& range_boundary,
  const view_1d<const Pack>& x1,
  const view_1d<const Pack>& x2,
  const Int col) const
{
  auto x1s = ekat::scalarize(x1);
  auto begin_x1 = x1s.data();
  auto end_x1 = begin_x1 + m_km1;

  const int i = col == -1 ? team.league_rank() : col;
  Kokkos::parallel_for(range_boundary, [&] (Int k2) {
    for (int s = 0; s < Pack::n; ++s) {
      const Scalar x2_indv = x2(k2)[s];
      auto ub = upper_bound(begin_x1, end_x1, x2_indv);
      int x1_idx = ub - begin_x1;
      if (x1_idx > 0) {
        --x1_idx;
      }
      m_indx_map(i, k2)[s] = x1_idx;
    }
  });
}

} // namespace ekat
