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

  template<typename FromView>
  ViewBroadcast (const FromView& from_v,
                 const std::vector<int>& extents)
  {
    constexpr int from_rank = FromView::rank();
    constexpr int to_rank = ToView::rank();

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
    typename ToView::traits::array_layout impl_layout,shape_layout;
    int ifrom = 0;
    for (int i=0; i<to_rank; ++i) {
      if (extents[i]<=0) {
        EKAT_REQUIRE_MSG (ifrom<from_rank,
            "Error! Too many missing extents in input vector.\n");
        impl_layout.dimension[i] = from_v.extent_int(ifrom);
        shape_layout.dimension[i] = from_v.extent_int(ifrom);
        coeff[i] = 1;
        ++ifrom;
      } else {
        impl_layout.dimension[i] = 1;
        shape_layout.dimension[i] = extents[i];
        coeff[i] = 0;
      }
    }
    EKAT_REQUIRE_MSG (ifrom==from_rank,
        "Error! Too many positive extents in input vector.\n");

    m_view_impl = view_type(from_v.data(),impl_layout);
    m_view_shape = view_type(from_v.data(),shape_layout);
  }

  KOKKOS_INLINE_FUNCTION
  int extent(int i) { return m_view_shape.extent(i); }

  template<typename... Is>
  KOKKOS_FORCEINLINE_FUNCTION
  reference_type operator()(Is... indices) const {
    int i=0;
    ((indices *= coeff[i++]),...);
    return m_view_impl(indices...);
  }

protected:

 view_type  m_view_impl;
 view_type  m_view_shape;

 int coeff[8];
};

} // namespace ekat

#endif // EKAT_BROADCAST_VIEW_HPP
