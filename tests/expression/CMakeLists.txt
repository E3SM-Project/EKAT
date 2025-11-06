include(EkatCreateUnitTest)

# Test expression templates
EkatCreateUnitTest(expressions expressions.cpp
  LIBS ekat::Expression)
