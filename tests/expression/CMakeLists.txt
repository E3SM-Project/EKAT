include(EkatCreateUnitTest)

# Test expression templates
EkatCreateUnitTest(expressions
  SOURCES expressions.cpp
  LIBS ekat::Expression)
