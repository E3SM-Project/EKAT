include(EkatCreateUnitTest)

# Test expression templates
EkatCreateUnitTest(expressions expressions.cpp
  LIBS ekat::Expression)

EkatCreateUnitTest(assignment_expressions assignment_expressions.cpp
  LIBS ekat::Expression)
