#ifndef EKAT_LIN_INTERP_HPP
#include "ekat_lin_interp.hpp"
#endif

#include "ekat_team_policy_utils.hpp"

namespace ekat {

// Never include this header directly, only ekat_lin_interp.hpp should include it

template <typename ScalarT, int PackSize, typename DeviceT>
LinInterp<ScalarT, PackSize, DeviceT>::LinInterp(int ncol, int km1, int km2) :
  m_km1(km1),
  m_km2(km2),
  m_km1_pack(ekat::npack<Pack>(km1)),
  m_km2_pack(ekat::npack<Pack>(km2)),
  m_policy(PolicyFactory<ExeSpace>::get_default_team_policy(ncol, m_km2_pack)),
  m_indx_map("m_indx_map", ncol, ekat::npack<IntPack>(km2))
{}

template <typename ScalarT, int PackSize, typename DeviceT>
template<typename V1, typename V2>
KOKKOS_INLINE_FUNCTION
void LinInterp<ScalarT, PackSize, DeviceT>::setup(
  const MemberType& team,
  const V1& x1,
  const V2& x2,
  const int col) const
{
  setup_impl(team, Kokkos::TeamVectorRange(team, m_km2_pack),
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
  const int col) const
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
  const int col) const
{
  lin_interp_impl(team,
                  Kokkos::TeamVectorRange(team, m_km2_pack),
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
  const int col) const
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
  const view_1d<const Pack>& x1,
  const view_1d<const Pack>& x2,
  const view_1d<const Pack>& y1,
  const view_1d<      Pack>& y2,
  const int col) const
{
  constexpr int N = Pack::n;
  using IPackT = ekat::Pack<int,N>;

  auto x1s = scalarize(x1);
  auto y1s = scalarize(y1);

  const int i = col == -1 ? team.league_rank() : col;

  Kokkos::parallel_for(range_boundary, [&] (int k2) {
    Pack x1_k1, x1_k1ph, y1_k1, y1_k1ph;
    IPackT k1ph;

    // Basic formula is y2(k2) = y1(k1) + m * (x2(k2)-x1(k1))
    // where m = (y1(k1+1)-y2(k1))/(x1(k1+1)-x1(k1)) is the slope of the line
    // going through (x1(k1),y1(k1)) and (x1(k1+1),y1(k1+1)).
    // The catch is that k1 may be the last point. In that case, we go backward instead of fwd
    // to compute the slope

    const auto& k1 = m_indx_map(i, k2);

    // k1ph = k1+h, where h=1 except at the last entry, where h=-1
    k1ph = k1;
    vector_simd
    for (int i=0; i<N; ++i) {
      // k1ph[i] = k1[i];
      if (k1ph[i]==(m_km1-1)) {
        --k1ph[i];
      } else {
        ++k1ph[i];
      }
    }

    // Eval x1 and y1 at k1 and k1+h
    // NOTE: 4 separate loops seem to be a few % better. Might have something to do with loop unrolling
    //       Also, using vector_simd deteriorates performance by >10%. That's not too surprising,
    //       considering that we are accessing non-contiguous memory. SIMD *forces* the compiler to
    //       vectorize, even if it would deem the vectorization inefficient
    for (int i=0; i<N; ++i) {
      x1_k1[i] = x1s(k1[i]);
    }
    for (int i=0; i<N; ++i) {
      y1_k1[i] = y1s(k1[i]);
    }
    for (int i=0; i<N; ++i) {
      x1_k1ph[i] = x1s(k1ph[i]);
    }
    for (int i=0; i<N; ++i) {
      y1_k1ph[i] = y1s(k1ph[i]);
    }

    // Apply linear interpolation formula. Use multiple statements with op= to minimize temporaries
    auto& y2_k2 = y2(k2);
    y2_k2  = y1_k1ph-y1_k1;
    y2_k2 *= x2(k2)-x1_k1;
    y2_k2 /= x1_k1ph-x1_k1;
    y2_k2 += y1_k1;
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
  const int col) const
{
  auto x1s = ekat::scalarize(x1);
  auto begin_x1 = x1s.data();
  auto end_x1 = begin_x1 + m_km1;

  const int i = col == -1 ? team.league_rank() : col;
  Kokkos::parallel_for(range_boundary, [&] (int k2) {
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
