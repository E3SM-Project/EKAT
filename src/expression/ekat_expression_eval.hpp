#ifndef EKAT_EXPRESSION_EVAL_HPP
#define EKAT_EXPRESSION_EVAL_HPP

#include "ekat_expression_base.hpp"
#include "ekat_assert.hpp"

namespace ekat {

namespace impl {

template<typename Derived, typename ViewT>
void evaluate (const Expression<Derived>& e, const ViewT& result)
{
  constexpr int N = ViewT::rank;
  static_assert(N>=1 and N<=3, "Unsuppoerted rank.\n");

  using dev_t = typename ViewT::traits::device_type;
  using exec_space = typename dev_t::execution_space;
  using Policy1D = Kokkos::RangePolicy<exec_space>;
  using PolicyMD = Kokkos::MDRangePolicy<exec_space,Kokkos::Rank<N>>;

  int beg[N] = {0};
  int end[N];
  for (int i=0; i<N; ++i) end[i] = result.extent(i);

  // Cast now, and capture the derived obj in the lambda, to make sure we get the
  // correct default copy constructor behavior
  auto impl = e.cast();
  if constexpr (N==1) {
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
  } else {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k) {
      result(i,j,k) = impl.eval(i,j,k);
    };
    Kokkos::parallel_for(p,eval);
  }
}

} // namespace impl

template<typename Derived, typename ViewT>
void evaluate (const Expression<Derived>& e, const ViewT& result)
{
  using value_t = typename ViewT::element_type;
  EKAT_REQUIRE_MSG (ViewT::rank==e.num_indices(),
    "[evaluate] Error! Input expression and result view have different ranks.\n"
    " - view rank: " + std::to_string(ViewT::rank) + "\n"
    " - expression rank: " + std::to_string(e.num_indices()) + "\n");

  EKAT_REQUIRE_MSG ((std::is_same_v<value_t,Real>),
    "[evaluate] Error! We currently only support expression templates for real-valued views.\n"
    " - view value type : " + std::string(typeid(value_t).name()) + "\n");

  impl::evaluate(e,result);
}

} // namespace ekat

#endif // EKAT_EXPRESSION_EVAL_HPP
