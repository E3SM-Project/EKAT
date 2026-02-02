#ifndef EKAT_MD_RANGE_HPP
#define EKAT_MD_RANGE_HPP

namespace ekat {

// Outermost kernel
template<typename MDPolicy, typename Functor>
void parallel_for (const MDPolicy& p, const Functor& f)
{
  using exec_space = typename MDPolicy::traits::execution_space;
  constexpr int N = MDPolicy::N;
  if constexpr (N==1) {
    Kokkos::RangePolicy<exec_space> p(p.m_lower[0],p.m_upper[0]),
    Kokkos::parallel_for(p, KOKKOS_LAMBDA(int i) {
      f(i);
    }):
  } else if constexpr (N==2) {
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
  } else if constexpr (N==3) {
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
  } else if constexpr (N==4) {
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
    EKAT_ERROR_MSG ("Error! Unsupported MDRange N (" + std::to_string(N) + ").\n");
  }
}

// Team-vector range
template<typename TeamMember, int N, typename Functor>
KOKKOS_INLINE_FUNCTION
void md_team_vector_range_for (const TeamMember& t, int lower[N], int upper[N], const Functor& f)
{
  if constexpr (N==1) {
    int dim0 = upper[0] - lower[0];
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim0),[&](int i) {
      f(lower[0] + i);
    });
  } else if constexpr (N==2) {
    int dim0 = upper[0] - lower[0];
    int dim1 = upper[1] - lower[1];
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim0*dim1),[&](int idx) {
      int i = lower[0] + idx / dim1;
      int j = lower[1] + idx % dim1;
      f(i,j);
    });
  } else if constexpr (N==3) {
    int dim0 = upper[0] - lower[0];
    int dim1 = upper[1] - lower[1];
    int dim2 = upper[2] - lower[2];
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim0*dim1*dim2),[&](int idx) {
      int i = lower[0] + (idx / dim2) / dim1;
      int j = lower[1] + (idx / dim2) % dim1;
      int k = lower[2] +  idx % dim2;
      f(i,j,k);
    });
  } else if constexpr (N==4) {
    int dim0 = upper[0] - lower[0];
    int dim1 = upper[1] - lower[1];
    int dim2 = upper[2] - lower[2];
    int dim3 = upper[3] - lower[3];
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim0*dim1*dim2*dim3),[&](int idx) {
      int i = lower[0] + ((idx / dim3) / dim2) / dim1;
      int j = lower[1] + ((idx / dim3) / dim2) % dim1;
      int k = lower[2] +  (idx / dim3) % dim2;
      int l = lower[3] +   idx % dim3;
      f(i,j,k,l);
    });
  } else {
    EKAT_KERNEL_ERROR_MSG ("Error! Unsupported rank for md_team_vector_range_for.");
  }
};

// Team-vector range with lower bound all zeros
template<typename TeamMember, int N, typename Functor>
KOKKOS_INLINE_FUNCTION
void md_team_vector_range_for (const TeamMember& t, int dim[N], const Functor& f)
{
  if constexpr (N==1) {
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim[0]),[&](int i) {
      f(i);
    });
  } else if constexpr (N==2) {
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim[0]*dim[1]),[&](int idx) {
      int i = idx / dim[1];
      int j = idx % dim[1];
      f(i,j);
    });
  } else if constexpr (N==3) {
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim[0]*dim[1]*dim[2]),[&](int idx) {
      int i = (idx / dim[2]) / dim[1];
      int j = (idx / dim[2]) % dim[1];
      int k =  idx % dim[2];
      f(i,j,k);
    });
  } else if constexpr (N==4) {
    Kokkos::parallel_for(Kokkos::TeamVectorRange(t,dim[0]*dim[1]*dim[2]*dim[3]),[&](int idx) {
      int i = ((idx / dim[3]) / dim[2]) / dim[1];
      int j = ((idx / dim[3]) / dim[2]) % dim[1];
      int k =  (idx / dim[3]) % dim[2];
      int l =   idx % dim[3];
      f(i,j,k,l);
    });
  } else {
    EKAT_KERNEL_ERROR_MSG ("Error! Unsupported rank for md_team_vector_range_for.");
  }
};

#endif // EKAT_MD_RANGE_HPP
