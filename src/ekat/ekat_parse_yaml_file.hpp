#ifndef EKAT_PARSE_YAML_FILE_HPP
#define EKAT_PARSE_YAML_FILE_HPP

#include "ekat/ekat_parameter_list.hpp"
#include <string>

namespace ekat {

ParameterList parse_yaml_file (const std::string& fname);
void parse_yaml_file (const std::string& fname, ParameterList& params);

} // namespace ekat

#endif // EKAT_PARSE_YAML_FILE_HPP
