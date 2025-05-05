include(EkatCreateUnitTest)

# Test yaml parser
EkatCreateUnitTest(yaml_parser yaml_parser.cpp
  LIBS YamlParser)
