#ifndef GENERATE_CONSTANTS_HPP
#define GENERATE_CONSTANTS_HPP

#include <string>
#include <map>
#include <iostream>
#include <string>
#include "yaml-cpp/yaml.h"

namespace ekat {
namespace physics {

typedef double ConstantsRealType;

class YamlException : public std::exception {
  public:
    YamlException(const std::string& msg) : _message(msg) {}

    YamlException(const char* fmt, ...);

    const char* what() const throw() {return _message.c_str();}
  private:
    std::string _message;
};

template <typename ValueType>
struct PhysicalConstant {

  static_assert(std::is_floating_point<ValueType>::value , "floating point type required.");

  std::string name;
  std::string software_name;
  ValueType value;
  std::string units;
  std::vector<std::string> src_info;

  PhysicalConstant(const std::string& n,
                   const std::string& c,
                   const ValueType& val,
                   const std::string& u,
                   const std::vector<std::string>& src) :
    name(n),
    software_name(c),
    value(val),
    units(u),
    src_info(src) {}
};

template <typename ValueType>
std::ostream& operator << (std::ostream& os, const PhysicalConstant<ValueType>& pc) {
  os << pc.name << ":\n"
     << "\tsoftwware_name: " << pc.software_name << "\n"
     << "\tvalue: " << pc.value << "\n"
     << "\tunits: " << pc.units << "\n"
     << "\tsource:\n";
  for (const auto& s : pc.src_info) {
    os << "\t\t" << s << "\n";
  }
  return os;
}

class ConstantsGenerator {
  public:

    std::map<std::string, PhysicalConstant<ConstantsRealType>> base_consts;
    std::string filename;

    std::string hpp_source() const;
    std::string cpp_source() const;
    std::string f90_source() const;

    explicit ConstantsGenerator(const std::string& cfile);

  private:
    YAML::Node root;
    void read_yaml_base_constants();
};


} // namespace physics
} // namespace ekat
#endif
