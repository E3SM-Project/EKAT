#ifndef EKAT_EXPRESSION_REDUCE_HPP
#define EKAT_EXPRESSION_REDUCE_HPP

#include "ekat_expression_meta.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

enum class ReduceOp {
  Sum,  // sum of all elements
  Max,  // maximum element
  Min,  // minimum element
  And,  // logical AND of all elements (all-true test)
  Or,   // logical OR  of all elements (any-true test)
};

// ReduceExpression reduces a rank-1 sub-expression to a rank-0 scalar.
//
// Usage pattern:
//   // Before the team kernel, allocate storage (one slot per league member):
//   Kokkos::View<double*> tmp("reduce_tmp", league_size);
//
//   // Inside the kernel:
//   auto expr = reduce_sum(view_expression(row_view), tmp) * view_expression(w);
//   evaluate(expr, team, result);  // two-phase: setup then elemental
//
// The setup() method performs the reduction and stores the result in
// tmp(team.league_rank()).  eval() reads from that slot and is safe to call
// from any number of threads after setup() completes.
template<typename EArg, ReduceOp OP>
class ReduceExpression {
public:
  static_assert(is_expr_v<EArg>,
    "[ReduceExpression] The sub-expression must be an Expression type.\n");
  static_assert(EArg::rank() == 1,
    "[ReduceExpression] ReduceExpression only supports rank-1 sub-expressions.\n");

  using sub_t   = EArg;
  using eval_t  = eval_return_t<EArg>;
  using store_t = Kokkos::View<eval_t*>;

  // Result is a scalar (rank-0)
  static constexpr int rank () { return 0; }
  KOKKOS_INLINE_FUNCTION int extent (int) const { return 0; }

  static constexpr ExprKind kind () { return ExprKind::Reduction; }

  KOKKOS_INLINE_FUNCTION
  ReduceExpression (const EArg& sub, const store_t& result)
    : m_sub(sub), m_result(result)
  {}

  // Phase-1: perform the reduction inside a team kernel.
  // All team members must call this (it is a collective operation).
  // After setup() returns every thread sees the updated result.
  template<typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void setup (const MemberType& team) {
    // If the sub-expression itself needs setup (nested reductions), do it first.
    if constexpr (EArg::kind() != ExprKind::Elemental) {
      m_sub.setup(team);
    }

    m_league_rank = team.league_rank();
    const int n   = m_sub.extent(0);
    eval_t result{};

    if constexpr (OP == ReduceOp::Sum) {
      Kokkos::parallel_reduce(
        Kokkos::TeamVectorRange(team, n),
        [&](int i, eval_t& val) { val += m_sub.eval(i); },
        result);
    } else if constexpr (OP == ReduceOp::Max) {
      Kokkos::parallel_reduce(
        Kokkos::TeamVectorRange(team, n),
        [&](int i, eval_t& val) {
          const eval_t v = m_sub.eval(i);
          if (v > val) val = v;
        },
        Kokkos::Max<eval_t>(result));
    } else if constexpr (OP == ReduceOp::Min) {
      Kokkos::parallel_reduce(
        Kokkos::TeamVectorRange(team, n),
        [&](int i, eval_t& val) {
          const eval_t v = m_sub.eval(i);
          if (v < val) val = v;
        },
        Kokkos::Min<eval_t>(result));
    } else if constexpr (OP == ReduceOp::And) {
      Kokkos::parallel_reduce(
        Kokkos::TeamVectorRange(team, n),
        [&](int i, eval_t& val) { val = val && m_sub.eval(i); },
        Kokkos::LAnd<eval_t>(result));
    } else { // ReduceOp::Or
      Kokkos::parallel_reduce(
        Kokkos::TeamVectorRange(team, n),
        [&](int i, eval_t& val) { val = val || m_sub.eval(i); },
        Kokkos::LOr<eval_t>(result));
    }

    // Write result from a single thread, then synchronize so all threads
    // can safely call eval() afterward.
    Kokkos::single(Kokkos::PerTeam(team), [&]() {
      m_result(m_league_rank) = result;
    });
    team.team_barrier();
  }

  // Phase-2 (elemental part): return the pre-computed scalar.
  // Valid only after setup() has been called for the current team.
  KOKKOS_INLINE_FUNCTION
  eval_t eval () const { return m_result(m_league_rank); }

private:
  EArg    m_sub;
  store_t m_result;
  int     m_league_rank = -1;
};

// Specialize meta utils
template<typename EArg, ReduceOp OP>
struct is_expr<ReduceExpression<EArg,OP>> : std::true_type {};
template<typename EArg, ReduceOp OP>
struct eval_return<ReduceExpression<EArg,OP>> {
  using type = typename ReduceExpression<EArg,OP>::eval_t;
};

} // namespace ekat

#endif // EKAT_EXPRESSION_REDUCE_HPP
