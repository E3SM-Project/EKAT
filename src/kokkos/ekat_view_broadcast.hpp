#ifndef EKAT_BROADCAST_VIEW_HPP
#define EKAT_BROADCAST_VIEW_HPP

#include "ekat_kokkos_meta.hpp"
#include "ekat_assert.hpp"

namespace ekat {

/*
 * Broadcast a view into a higher dimensional view
 *
 * This allows to replicate (e.g. extrude) a view as a higher
 * dimensional one, without having to manually copy entries.
 * E.g., one can replicate a column vector v into a matrix A
 * with each column equal to v, so that A(i,j) = v(j) for every i.
 */

template<typename ToView>
class ViewBroadcast
{
public:
  using view_type  = Unmanaged<ToView>;
  using reference_type = typename view_type::reference_type;

  ViewBroadcast () = default;

  template<typename FromView>
  ViewBroadcast (const FromView& from_v,
                 const std::vector<int>& extents)
  {
    setup(from_v,extents);
  }

  // Broadcast an input view to the type provided by the class template arg
  //  - from_v: view to be broadcasted
  //  - extents: list of extents of the outfacing view (of type ToView).
  //    Must have rank=ToView::rank(). Must contain FromView::rank() entries
  //    that are <=0, which signal that those dimensions' extents will be
  //    retrieved from the input view.
  template<typename FromView>
  void setup (const FromView& from_v,
              const std::vector<int>& extents)
  {
    constexpr int from_rank = FromView::rank();
    constexpr int to_rank = ToView::rank();

    EKAT_REQUIRE_MSG (from_rank>=1,
        "[ViewBroadcast] Error! FromView rank must be at least 1.\n"
        " FromView::rank(): " << FromView::rank() << "\n");
    EKAT_REQUIRE_MSG (from_rank<=to_rank,
        "[ViewBroadcast] Error! FromView rank exceeds ToView rank.\n"
        " FromView::rank(): " << FromView::rank() << "\n"
        " ToView::rank(): " << ToView::rank() << "\n");
    EKAT_REQUIRE_MSG (to_rank<=8,
        "[ViewBroadcast] Error! ToView rank exceeds maximum rank (8).\n"
        " ToView::rank(): " << ToView::rank() << "\n");

    EKAT_REQUIRE_MSG (extents.size()==to_rank,
        "[ViewBroadcast] Badly sized extents vector.\n");

    // Init all dims as "ignored", then set up the ones actually picked
    typename ToView::traits::array_layout impl_layout;
    int ifrom = 0;
    for (int i=0; i<to_rank; ++i) {
      if (extents[i]<=0) {
        EKAT_REQUIRE_MSG (ifrom<from_rank,
            "Error! Too many missing extents in input vector.\n");
        impl_layout.dimension[i] = from_v.extent_int(ifrom);
        m_shape[i] = from_v.extent_int(ifrom);
        m_coeff[i] = 1;
        ++ifrom;
      } else {
        impl_layout.dimension[i] = 1;
        m_shape[i] = extents[i];
        m_coeff[i] = 0;
      }
    }
    EKAT_REQUIRE_MSG (ifrom==from_rank,
        "Error! Too many positive extents in input vector.\n");

    m_view_impl = view_type(from_v.data(),impl_layout);
  }

  KOKKOS_INLINE_FUNCTION
  int extent(int i) {
    EKAT_KERNEL_ASSERT_MSG (i>=0 and i<=m_view_impl.rank(),
        "[ViewBroadcast::extent] Error! Index out of bounds.\n");
    return m_shape[i];
  }

  // Rank 1
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0) const {
    return m_view_impl(i0*m_coeff[0]);
  }

  // Rank 2
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0, IntType i1) const {
    return m_view_impl(i0*m_coeff[0],i1*m_coeff[1]);
  }

  // Rank 3
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0, IntType i1, IntType i2) const {
    return m_view_impl(i0*m_coeff[0],i1*m_coeff[1],i2*m_coeff[2]);
  }

  // Rank 4
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0, IntType i1, IntType i2, IntType i3) const {
    return m_view_impl(i0*m_coeff[0],i1*m_coeff[1],i2*m_coeff[2],i3*m_coeff[3]);
  }

  // Rank 5
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0, IntType i1, IntType i2, IntType i3,
             IntType i4) const {
    return m_view_impl(i0*m_coeff[0],i1*m_coeff[1],i2*m_coeff[2],i3*m_coeff[3],
                       i4*m_coeff[4]);
  }

  // Rank 6
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0, IntType i1, IntType i2, IntType i3,
             IntType i4, IntType i5) const {
    return m_view_impl(i0*m_coeff[0],i1*m_coeff[1],i2*m_coeff[2],i3*m_coeff[3],
                       i4*m_coeff[4],i5*m_coeff[5]);
  }

  // Rank 7
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0, IntType i1, IntType i2, IntType i3,
             IntType i4, IntType i5, IntType i6) const {
    return m_view_impl(i0*m_coeff[0],i1*m_coeff[1],i2*m_coeff[2],i3*m_coeff[3],
                       i4*m_coeff[4],i5*m_coeff[5],i6*m_coeff[6]);
  }

  // Rank 8
  template<typename IntType>
  KOKKOS_FORCEINLINE_FUNCTION
  std::enable_if_t<std::is_integral_v<IntType>,reference_type>
  operator()(IntType i0, IntType i1, IntType i2, IntType i3,
             IntType i4, IntType i5, IntType i6, IntType i7) const {
    return m_view_impl(i0*m_coeff[0],i1*m_coeff[1],i2*m_coeff[2],i3*m_coeff[3],
                       i4*m_coeff[4],i5*m_coeff[5],i6*m_coeff[6],i7*m_coeff[7]);
  }

protected:

 view_type  m_view_impl;

 int m_shape[8];
 int m_coeff[8];
};


template<typename ToView,typename FromView>
ViewBroadcast<ToView> broadcast (const FromView& from,
                                 const std::vector<int>& extents)
{
  return ViewBroadcast<ToView>(from,extents);
}

} // namespace ekat

#endif // EKAT_BROADCAST_VIEW_HPP
