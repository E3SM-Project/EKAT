#ifndef EKAT_VIEW_UTILS_HPP
#define EKAT_VIEW_UTILS_HPP

#include "ekat_kokkos_meta.hpp"
#include "ekat_type_traits.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

// Helper for getting correct layout
template<class ViewT, class... Extents>
requires (std::is_same_v<typename ViewT::traits::array_layout,Kokkos::LayoutStride>)
KOKKOS_INLINE_FUNCTION static Kokkos::LayoutStride get_layout(const Extents... exts) {
  constexpr int rank = sizeof...(exts);
  assert(rank == ViewT::rank());

  // EKAT is assuming layout stride is equiv to layout right
  int dims[] = {exts...};
  int order[rank];
  for (std::size_t r=0; r<rank; ++r) {
      order[r] = rank - 1 - r; // Fill with descending values
  }

  return Kokkos::LayoutStride::order_dimensions(rank, order, dims);
}

template<class ViewT, class... Extents>
requires (std::is_same_v<typename ViewT::traits::array_layout, Kokkos::LayoutRight>)
KOKKOS_INLINE_FUNCTION static Kokkos::LayoutRight get_layout(const Extents... exts) {
  constexpr int rank = sizeof...(exts);
  assert(rank == ViewT::rank());

  return Kokkos::LayoutRight(exts...);
}

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

template <typename ViewType, typename rngAlg, typename PDF>
typename std::enable_if<Kokkos::is_view<ViewType>::value, void>::type
genRandArray(ViewType view, rngAlg &engine, PDF &&pdf) {
  auto mirror = Kokkos::create_mirror_view(view);
  for (size_t i = 0; i < mirror.size(); ++i) {
    mirror.data()[i] = pdf(engine);
  }
  Kokkos::deep_copy(view, mirror);
}

template<typename ViewType>
typename ViewType::host_mirror_type
create_host_mirror_and_copy (const ViewType& v)
{
  auto vh = Kokkos::create_mirror_view(v);
  Kokkos::deep_copy(vh,v);
  return vh;
}

} // namespace ekat

#endif // EKAT_VIEW_UTILS_HPP
