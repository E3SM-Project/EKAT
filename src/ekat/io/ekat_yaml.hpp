#ifndef EKAT_YAML_HPP
#define EKAT_YAML_HPP

#include "ekat/ekat_parameter_list.hpp"
#include <string>

namespace ekat {

ParameterList parse_yaml_file (const std::string& fname);
void parse_yaml_file (const std::string& fname, ParameterList& params);

void write_yaml_file (const std::string& fname, const ParameterList& params);

} // namespace ekat

#endif // EKAT_YAML_HPP
