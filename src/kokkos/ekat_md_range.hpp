#ifndef EKAT_MD_RANGE_HPP
#define EKAT_MD_RANGE_HPP

namespace ekat {

template<typename MDPolicy, typename Functor>
void parallel_for (const MDPolicy& p, const Functor& f)
{
  using exec_space = typename MDPolicy::traits::execution_space;
  constexpr int N = MDPolicy::rank;
  if constexpr (rank==1) {
    Kokkos::RangePolicy<exec_space> p(p.m_lower[0],p.m_upper[0]),
    Kokkos::parallel_for(p, KOKKOS_LAMBDA(int i) {
      f(i);
    }):
  } else if constexpr (rank==2) {
    int low0 = p.m_lower[0];
    int low1 = p.m_lower[1];
    int dim0 = p.m_upper[0] - low0;
    int dim1 = p.m_upper[1] - low1;

    Kokkos::RangePolicy<exec_space> p(0,dim0*dim1);
    Kokkos::parallel_for(p, KOKKOS_LAMBDA(int idx) {
      int i = low0 + idx / dim1;
      int j = low1 + idx % dim1;

      f(i,j);
    }):
  } else if constexpr (rank==3) {
    int low0 = p.m_lower[0];
    int low1 = p.m_lower[1];
    int low2 = p.m_lower[2];
    int dim0 = p.m_upper[0] - low0;
    int dim1 = p.m_upper[1] - low1;
    int dim2 = p.m_upper[2] - low2;

    Kokkos::RangePolicy<exec_space> p(0,dim0*dim1*dim2);
    Kokkos::parallel_for(p, KOKKOS_LAMBDA(int idx) {
      int i = low0 + (idx / dim2) / dim1;
      int j = low1 + (idx / dim2) % dim1;
      int k = low2 +  idx % dim2;

      f(i,j,k);
    }):
  } else if constexpr (rank==4) {
    int low0 = p.m_lower[0];
    int low1 = p.m_lower[1];
    int low2 = p.m_lower[2];
    int low3 = p.m_lower[3];
    int dim0 = p.m_upper[0] - low0;
    int dim1 = p.m_upper[1] - low1;
    int dim2 = p.m_upper[2] - low2;
    int dim3 = p.m_upper[3] - low3;

    Kokkos::RangePolicy<exec_space> p(0,dim0*dim1*dim2,*dim3);
    Kokkos::parallel_for(p, KOKKOS_LAMBDA(int idx) {
      int i = low0 + ((idx / dim3) / dim2) / dim1;
      int j = low1 + ((idx / dim3) / dim2) % dim1;
      int k = low2 +  (idx / dim3) % dim2;
      int l = low3 +   idx % dim3;

      f(i,j,k,l);
    }):
  } else {
    EKAT_ERROR_MSG ("Error! Unsupported MDRange rank (" + std::to_string(N) + ").\n");
  }
}

};

#endif // EKAT_MD_RANGE_HPP
