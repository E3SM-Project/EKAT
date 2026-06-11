include(EkatCreateUnitTest)

# Test expression templates
EkatCreateUnitTest(expressions
  SOURCES expressions.cpp
  LIBS ekat::Expression)

# Test expression templates created/evaluated inside range-type parallel loops
EkatCreateUnitTest(range_expressions
  SOURCES range_expressions.cpp
  LIBS ekat::Expression)
