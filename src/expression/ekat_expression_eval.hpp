#ifndef EKAT_EXPRESSION_EVAL_HPP
#define EKAT_EXPRESSION_EVAL_HPP

#include "ekat_expression_base.hpp"
#include "ekat_assert.hpp"

namespace ekat {

template<typename Derived, typename ViewT>
void evaluate (const Expression<Derived>& e, const ViewT& result)
{
  constexpr int N = ViewT::rank;

  EKAT_REQUIRE_MSG (N==Derived::rank(),
    "[evaluate] Error! Input expression and result view have different ranks.\n"
    " - view rank: " + std::to_string(N) + "\n"
    " - expression rank: " + std::to_string(Derived::rank()) + "\n");

  // Kokkos views don't go higher than rank 8, but just in case...
  static_assert(N<=8, "[evaluate] Unsupported expression rank.\n");

  using dev_t = typename ViewT::traits::device_type;
  using exec_space = typename dev_t::execution_space;
  using Policy1D = Kokkos::RangePolicy<exec_space>;
  using PolicyMD = Kokkos::MDRangePolicy<exec_space,Kokkos::Rank<N>>;

  // Ensure the beg/end array size is > 0. While compilers may allow size-0 arrays as an extension,
  // it is not standard compliant. For N=0, we won't use these anyways...
  int beg[N==0 ? 1 : N] = {};
  int end[N==0 ? 1 : N] = {};
  for (int i=0; i<N; ++i) {
    EKAT_REQUIRE_MSG (e.extent(i)==result.extent_int(i),
      "[evaluate] Error! Input expression and output view have incompatible extents.\n");
    end[i] = e.extent(i);
  }

  // Cast now, and capture the derived obj in the lambda, to make sure we get the
  // correct default copy constructor behavior
  auto impl = e.cast();
  if constexpr (N==0) {
    Policy1D p(0,1);
    auto eval = KOKKOS_LAMBDA (int) {
      result() = impl.eval();
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==1) {
    Policy1D p(0,end[0]);
    auto eval = KOKKOS_LAMBDA (int i) {
      result(i) = impl.eval(i);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==2) {

    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j) {
      result(i,j) = impl.eval(i,j);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==3) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k) {
      result(i,j,k) = impl.eval(i,j,k);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==4) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l) {
      result(i,j,k,l) = impl.eval(i,j,k,l);
    };
  } else if constexpr (N==5) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m) {
      result(i,j,k,l,m) = impl.eval(i,j,k,l,m);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==6) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m,int n) {
      result(i,j,k,l,m,n) = impl.eval(i,j,k,l,m,n);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==7) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m,int n,int o) {
      result(i,j,k,l,m,n,o) = impl.eval(i,j,k,l,m,n,o);
    };
    Kokkos::parallel_for(p,eval);
  } else {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m,int n,int o,int p) {
      result(i,j,k,l,m,n,o,p) = impl.eval(i,j,k,l,m,n,o,p);
    };
    Kokkos::parallel_for(p,eval);
  }
}

} // namespace ekat

#endif // EKAT_EXPRESSION_EVAL_HPP
