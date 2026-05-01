#ifndef EKAT_EXPRESSION_EVAL_HPP
#define EKAT_EXPRESSION_EVAL_HPP

#include "ekat_expression_meta.hpp"
#include "ekat_assert.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

// -----------------------------------------------------------------------
// Host-side evaluate: launch a top-level Kokkos::parallel_for
// -----------------------------------------------------------------------
template<typename Expression, typename ViewT>
std::enable_if_t<is_expr_v<Expression>>
evaluate (const Expression& e, const ViewT& result)
{
  constexpr int N = ViewT::rank;

  EKAT_REQUIRE_MSG (N==Expression::rank(),
    "[evaluate] Error! Input expression and result view have different ranks.\n"
    " - view rank: " + std::to_string(N) + "\n"
    " - expression rank: " + std::to_string(Expression::rank()) + "\n");

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
  if constexpr (N==0) {
    Policy1D p(0,1);
    auto eval = KOKKOS_LAMBDA (int) {
      result() = e.eval();
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==1) {
    Policy1D p(0,end[0]);
    auto eval = KOKKOS_LAMBDA (int i) {
      result(i) = e.eval(i);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==2) {

    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j) {
      result(i,j) = e.eval(i,j);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==3) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k) {
      result(i,j,k) = e.eval(i,j,k);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==4) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l) {
      result(i,j,k,l) = e.eval(i,j,k,l);
    };
  } else if constexpr (N==5) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m) {
      result(i,j,k,l,m) = e.eval(i,j,k,l,m);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==6) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m,int n) {
      result(i,j,k,l,m,n) = e.eval(i,j,k,l,m,n);
    };
    Kokkos::parallel_for(p,eval);
  } else if constexpr (N==7) {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m,int n,int o) {
      result(i,j,k,l,m,n,o) = e.eval(i,j,k,l,m,n,o);
    };
    Kokkos::parallel_for(p,eval);
  } else {
    PolicyMD p(beg,end);
    auto eval = KOKKOS_LAMBDA (int i,int j,int k,int l,int m,int n,int o,int p) {
      result(i,j,k,l,m,n,o,p) = e.eval(i,j,k,l,m,n,o,p);
    };
    Kokkos::parallel_for(p,eval);
  }
}

// -----------------------------------------------------------------------
// Team-parallel evaluate: call from inside a Kokkos::TeamPolicy kernel.
// -----------------------------------------------------------------------

// Controls how the team's threads and vector lanes are used to iterate
// over the output view.
enum class TeamEvalMode {
  // Distribute work across all threads and vectors in the team.
  // Good default for rank-1 expressions.
  TeamVectorRange,

  // Use TeamThreadRange for the first dimension and ThreadVectorRange for
  // the second dimension.  Good default for rank-2 expressions.
  TeamThread_ThreadVector,

  // Use ThreadVectorRange only (caller is already inside a TeamThreadRange
  // loop).  Works on rank-1 expressions.
  ThreadVectorRange,

  // Pick TeamVectorRange for rank <= 1, TeamThread_ThreadVector otherwise.
  Auto,
};

// evaluate(expr, team, result [, mode])
//
// Two-phase evaluation inside a team kernel:
//   Phase 1 (setup): traverse the expression tree and compute any
//     reductions or team-level pre-work (only when kind != Elemental).
//   Phase 2 (elemental): assign result(i[,j,...]) = expr.eval(i[,j,...])
//     in parallel using the chosen TeamEvalMode.
//
// The expression is taken by value so that setup() can modify mutable
// state (e.g. caching a reduction result) without affecting the caller's
// object.
template<typename Expression, typename MemberType, typename ViewT>
KOKKOS_INLINE_FUNCTION
std::enable_if_t<is_expr_v<Expression>>
evaluate (Expression e,
          const MemberType& team,
          const ViewT& result,
          TeamEvalMode mode = TeamEvalMode::Auto)
{
  constexpr int N = ViewT::rank;
  static_assert(N == Expression::rank(),
    "[evaluate] Error! Expression and result view have different ranks.");

  // --- Phase 1: setup (compute reductions, etc.) ---
  if constexpr (Expression::kind() != ExprKind::Elemental) {
    e.setup(team);
    // setup() inserts team barriers internally after each reduction; add
    // one final barrier here to guarantee all setup work is visible before
    // the elemental loop below.
    team.team_barrier();
  }

  // --- Phase 2: elemental loop ---
  TeamEvalMode actual = mode;
  if (mode == TeamEvalMode::Auto) {
    actual = (N <= 1) ? TeamEvalMode::TeamVectorRange
                      : TeamEvalMode::TeamThread_ThreadVector;
  }

  if (actual == TeamEvalMode::TeamVectorRange) {
    if constexpr (N == 0) {
      Kokkos::single(Kokkos::PerTeam(team), [&]() { result() = e.eval(); });
    } else if constexpr (N == 1) {
      Kokkos::parallel_for(Kokkos::TeamVectorRange(team, e.extent(0)),
        [&](int i) { result(i) = e.eval(i); });
    }
    // For N > 1 with TeamVectorRange, fall through to TeamThread_ThreadVector
    // (TeamVectorRange cannot express multi-dimensional loops directly).
    else {
      Kokkos::parallel_for(Kokkos::TeamThreadRange(team, e.extent(0)),
        [&](int i) {
          for (int j = 0; j < e.extent(1); ++j)
            Kokkos::parallel_for(Kokkos::ThreadVectorRange(team, e.extent(2 > N-1 ? N-1 : 2)),
              [&](int k) {
                if constexpr (N == 2) result(i,j)   = e.eval(i,j);
                else                  result(i,j,k)  = e.eval(i,j,k);
              });
        });
    }
  } else if (actual == TeamEvalMode::ThreadVectorRange) {
    // Caller is already inside a TeamThreadRange loop; cover the innermost dim.
    if constexpr (N == 1) {
      Kokkos::parallel_for(Kokkos::ThreadVectorRange(team, e.extent(0)),
        [&](int i) { result(i) = e.eval(i); });
    } else if constexpr (N == 2) {
      for (int i = 0; i < e.extent(0); ++i)
        Kokkos::parallel_for(Kokkos::ThreadVectorRange(team, e.extent(1)),
          [&](int j) { result(i,j) = e.eval(i,j); });
    }
  } else { // TeamThread_ThreadVector
    if constexpr (N == 1) {
      // Degenerate: no thread-level loop, just vector
      Kokkos::parallel_for(Kokkos::TeamVectorRange(team, e.extent(0)),
        [&](int i) { result(i) = e.eval(i); });
    } else if constexpr (N == 2) {
      Kokkos::parallel_for(Kokkos::TeamThreadRange(team, e.extent(0)),
        [&](int i) {
          Kokkos::parallel_for(Kokkos::ThreadVectorRange(team, e.extent(1)),
            [&](int j) { result(i,j) = e.eval(i,j); });
        });
    } else if constexpr (N == 3) {
      Kokkos::parallel_for(Kokkos::TeamThreadRange(team, e.extent(0)),
        [&](int i) {
          for (int j = 0; j < e.extent(1); ++j)
            Kokkos::parallel_for(Kokkos::ThreadVectorRange(team, e.extent(2)),
              [&](int k) { result(i,j,k) = e.eval(i,j,k); });
        });
    } else if constexpr (N == 4) {
      Kokkos::parallel_for(Kokkos::TeamThreadRange(team, e.extent(0)),
        [&](int i) {
          for (int j = 0; j < e.extent(1); ++j)
            for (int k = 0; k < e.extent(2); ++k)
              Kokkos::parallel_for(Kokkos::ThreadVectorRange(team, e.extent(3)),
                [&](int l) { result(i,j,k,l) = e.eval(i,j,k,l); });
        });
    }
    // Ranks 5–8 follow the same pattern but are uncommon in team kernels;
    // extend as needed.
  }
}

} // namespace ekat

#endif // EKAT_EXPRESSION_EVAL_HPP
