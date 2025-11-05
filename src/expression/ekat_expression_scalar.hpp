#ifndef EKAT_SCALAR_EXPRESSION_HPP
#define EKAT_SCALAR_EXPRESSION_HPP

#include "ekat_expression_base.hpp"

namespace ekat {

template<typename ST>
class ScalarExpression : public Expression<ScalarExpression<ST>> {
public:
  ScalarExpression (const ST value)
  {
    m_value = value;
  }

  int num_indices () const { return 0; }

  template<typename... Args>
  KOKKOS_INLINE_FUNCTION
  ST eval(Args...) const {
    return m_value;
  }

  static ST ret_type () { return 0; }
protected:

  ST m_value;
};

template<typename ST>
ScalarExpression<ST> scalar_expression(const ST v)
{
  return ScalarExpression<ST>(v);
}

} // namespace ekat

#endif // EKAT_SCALAR_EXPRESSION_HPP
