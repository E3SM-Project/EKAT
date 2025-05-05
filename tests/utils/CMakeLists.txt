include(EkatCreateUnitTest)

# Test std_meta stuff
EkatCreateUnitTest(std_meta std_meta.cpp
  LIBS Utils TestUtils)

# Test meta utilities
EkatCreateUnitTest(meta_utils meta_utils.cpp
  LIBS Utils TestUtils)

# Test parameter list
EkatCreateUnitTest(parameter_list parameter_list.cpp
  LIBS Utils TestUtils)

# Test type traits
EkatCreateUnitTest(type_traits_utils type_traits_utils.cpp
  LIBS Utils TestUtils)

# Test string utilities
EkatCreateUnitTest(string_utils string_utils.cpp
  LIBS Utils TestUtils)

# Test factory
EkatCreateUnitTest(factory factory.cpp
  LIBS Utils TestUtils)
