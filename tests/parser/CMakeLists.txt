include(EkatCreateUnitTest)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/input.yaml
                ${CMAKE_CURRENT_BINARY_DIR}/input.yaml COPYONLY)

# Test yaml parser
EkatCreateUnitTest(yaml_parser yaml_parser.cpp
  LIBS ekat::YamlParser)
