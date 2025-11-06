include(EkatCreateUnitTest)

# Test expression templates
EkatCreateUnitTest(expressions expressions.cpp
  LIBS ekat::Expression)

if (EKAT_ENABLE_PACK)
  # Test expression templates
  EkatCreateUnitTest(packed_expressions packed_expressions.cpp
    LIBS ekat::Expression ekat::Pack)
endif()
