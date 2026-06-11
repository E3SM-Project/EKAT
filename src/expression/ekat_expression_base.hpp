#ifndef EKAT_EXPRESSION_BASE_HPP
#define EKAT_EXPRESSION_BASE_HPP

#include "ekat_expression_traits.hpp"

#include <Kokkos_Core.hpp>

namespace ekat {

enum class LoopType {
  Range,
  League,
  Team,
  Thread
};

template<typename TeamMemberType = void>
struct EvalSpecs {
  KOKKOS_INLINE_FUNCTION
  EvalSpecs () = default;

  KOKKOS_INLINE_FUNCTION
  EvalSpecs (const LoopType loop_type_in,
             const TeamMemberType* team_in = nullptr,
             const int tp_rank_in = -1)
   : enclosing_loop (loop_type_in)
   , team(team_in)
   , tp_rank (tp_rank_in)
  {
#ifndef NDEBUG
    if (loop_type==LoopType::Range) {
      EKAT_KERNEL_REQUIRE_MSG(team==nullptr,
          "[EvalSpecs] Error! A Range loop type does not expect a valid TeamMemberType pointer.\n");
    } else {
      EKAT_KERNEL_REQUIRE_MSG(team!=nullptr,
          "[EvalSpecs] Error! A non-Range loop type requires a valid TeamMemberType pointer.\n");
      EKAT_KERNEL_REQUIRE_MSG(tp_rank>0 and tp_rank<=3,
          "[EvalSpecs] Error! Team policy rank is out of bounds.\n");
    }
#endif
  }

  // Specifies in which kind of loop the evaluation is happening
  LoopType  enclosing_loop = LoopType::Range;

  // If the loop type is not Range, this can be used for dispatching inner parallel loops
  const TeamMemberType* team = nullptr;

  // If inside a Hierarchical parallel loop, the TeamPolicy "rank" allows
  // expressions to figure out what kind of parallel loop (if any) they
  // can dispatch internally.
  int tp_rank;
};

template<typename TeamMemberType>
auto eval_specs(const TeamMemberType& team,
                const LoopType loop_type,
                const int tp_rank)
{
  return TeamEvalSpecs<TeamMemberType>(loop_type,team,tp_rank);
}

auto eval_specs()
{
  return TeamEvalSpecs<>(LoopType::Range);
}

// struct PolicySpecs {
//   int  league_size = -1;
//   int  team_size = Kokkos::AUTO();
//   int  vec_size = Kokkos::AUTO();
// };

// A base class for expressions
template<typename Derived>
class ExpressionBase {
public:
  using expression_tag = void; // Add tag to be used for SFINAE and meta-utils

  KOKKOS_INLINE_FUNCTION
  ExpressionBase () {
    static_assert(is_expr_v<Derived>,
        "Template arg is NOT an expression. Ensure Derived inherits from ExpressionBase.");
  }

  KOKKOS_INLINE_FUNCTION
  Derived& cast () { return *static_cast<Derived*>(this); }
  KOKKOS_INLINE_FUNCTION
  const Derived& cast () const { return *static_cast<const Derived*>(this); }

  ExpressionBase<Derived>& as_base () { return *this; }
  const ExpressionBase<Derived>& as_base () const { return *this; }

  static constexpr int rank() { return Derived::rank(); }

  KOKKOS_INLINE_FUNCTION
  int extent (int i) const { return cast().extent(i); }

  // int inner_parallel_layers const { return cast().inner_parallel_layers(); }
  // PolicySpecs get_policy_specs () const { return cast().get_policy_specs(); }

  template<typename TeamMember, typename... Args,
           typename = std::enable_if_t<(std::is_integral_v<Args> && ...)>>
  KOKKOS_INLINE_FUNCTION
  auto eval (const TeamEvalSpecs<TeamMember>& specs, Args... args) const
  {
    return cast().eval(specs, args...);
  }

  template<typename... Args,
           typename = std::enable_if_t<(std::is_integral_v<Args> && ...)>>
  KOKKOS_INLINE_FUNCTION
  auto eval (Args... args) const
  {
    return cast().eval(args...);
  }

  // template<typename Device>
  // typename KokkosTypes<Device>::TeamPolicy get_team_policy() const
  // {
  //   return cast().get_team_policy();
  // }
};

} // namespace ekat

#endif // EKAT_EXPRESSION_BASE_HPP
