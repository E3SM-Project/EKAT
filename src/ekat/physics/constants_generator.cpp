#include "ekat/physics/constants_generator.hpp"

#include <cstdarg>
#include <sstream>
#include <iomanip>
#include <limits>

namespace ekat {
namespace physics {

YamlException::YamlException(const char* fmt, ...) {
  char ss[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(ss, 255, fmt, args);
  va_end(args);
  _message.assign(ss);
}

ConstantsGenerator::ConstantsGenerator(const std::string& cfile) :
  filename(cfile) {
    try {
      root = YAML::LoadFile(filename);
    }
    catch (YAML::BadFile& e) {
      throw YamlException(e.what());
    }
    catch (YAML::ParserException& e) {
      throw YamlException(e.what());
    }
    read_yaml_base_constants();
}

void ConstantsGenerator::read_yaml_base_constants() {
  if (root["base_constants"] and root["base_constants"].IsMap()) {
    auto node = root["base_constants"];
    for (auto iter = node.begin(); iter != node.end(); ++iter) {
      const auto name = iter->first.as<std::string>();
      const auto mnode = iter->second;
      if (not mnode["software_name"]) {
        throw YamlException("%s: software_name not found.", name.c_str());
      }
      else if (not mnode["value"]) {
        throw YamlException("%s: value not found.", name.c_str());
      }
      else if (not mnode["units"]) {
        throw YamlException("%s: units not found.", name.c_str());
      }
      else if (not mnode["source"]) {
        throw YamlException("%s: source information not found.", name.c_str());
      }
      else {
        std::vector<std::string> src_info;
        auto srcnode = mnode["source"];
        if (srcnode["name"]) {
          src_info.push_back("name: " + srcnode["name"].as<std::string>());
        }
        if (srcnode["command"]) {
          src_info.push_back("command: " + srcnode["command"].as<std::string>());
        }
        if (srcnode["url"]) {
          src_info.push_back("url: " + srcnode["url"].as<std::string>());
        }
        if (srcnode["doi"]) {
          src_info.push_back("doi: " + srcnode["doi"].as<std::string>());
        }
        if (srcnode["citation"]) {
          src_info.push_back("citation: " + srcnode["citation"].as<std::string>());
        }
        if (srcnode["definition"]) {
          src_info.push_back("definition: " + srcnode["definition"].as<std::string>());
        }
        if (srcnode["note"]) {
          src_info.push_back("note: " + srcnode["note"].as<std::string>());
        }

        base_consts.emplace(name,
          PhysicalConstant<ConstantsRealType>(name,
              mnode["software_name"].as<std::string>(),
              mnode["value"].as<ConstantsRealType>(),
              mnode["units"].as<std::string>(),
              src_info)
              );
      }
    }
  }
  else {
    throw YamlException("%s: No base_constants section found.", filename.c_str());
  }
}

std::string ConstantsGenerator::hpp_source() const {
  std::ostringstream ss;
  ss << "#ifndef BASE_PHYSICAL_CONSTANTS_HPP\n#define BASE_PHYSICAL_CONSTANTS_HPP\n\n";
  ss << "#include <type_traits>\n\n";
  ss << "/*\n DO NOT EDIT\n This file was automatically generated from ekat/src/ekat/physics/constants.yaml\n\n";
  ss << "ALL CONSTANTS ARE DEFINED IN SI UNITS\n\n";
  ss << "\ttime:                seconds   [s]\n";
  ss << "\tlength:              meters    [m]\n";
  ss << "\tmass:                kilograms [kg]\n";
  ss << "\ttemperature          Kelvin    [K]\n";
  ss << "\tpressure             Pascals   [Pa]\n";
  ss << "\tforce                Newtons   [N]\n";
  ss << "\tamount of substance  Moles     [mol]\n";
  ss << "\twork                 Joules    [J]\n";
  ss << "*/\n\n";
  ss << "namespace ekat {\n";
  ss << "namespace physics {\n\n";
  ss << "template <typename Scalar>\n";
  ss << "struct BaseConstants {\n";
  ss << "\tstatic_assert(std::is_floating_point<Scalar>::value, \"floating point type required.\");\n\n";
  for (const auto& pc : base_consts ) {
    ss << "\t/*\n";
    ss << "\t" << pc.second.name << "\n";
    ss << "\tunits: " << pc.second.units << "\n";
    ss << "\t" << "source info:\n";
    for (const auto& s : pc.second.src_info) {
      ss << "\t\t" << s << "\n";
    }
    ss << "\t*/\n";
    ss << "\tstatic constexpr Scalar " << pc.second.software_name << " = ";
    ss << std::setprecision(std::numeric_limits<ConstantsRealType>::digits10 + 1);
    ss << pc.second.value << ";\n\n";
  }
  ss << "}; // end struct\n";
  ss << "} // namespace physics\n";
  ss << "} // namespace ekat\n";
  ss << "#endif\n";
  return ss.str();
}

std::string ConstantsGenerator::cpp_source() const {
  std::ostringstream ss;
  ss << "/* This file was generated by EKAT with generate_constants.hpp and\n"
     << " * generate_constants.cpp using data defined in \n"
     << " *         ekat/src/ekat/physics/constants.yaml \n"
     << " * \n"
     << " *      DO NOT EDIT\n"
     << " * \n"
     << "*/\n";
  ss << "#include \"ekat/physics/base_constants.hpp\"\n\n"
     << "namespace ekat {\n"
     << "namespace physics {\n";
  for (const auto& pc : base_consts) {
    ss << "  template <typename Scalar>\n"
       << "  Scalar constexpr BaseConstants<Scalar>::" << pc.second.software_name
       << ";\n";
  }
  ss << "// ETI\n"
     << "template struct BaseConstants<float>;\n"
     << "template struct BaseConstants<double>;\n";
  ss << "} // namespace physics\n"
     << "} // namespace ekat\n";
  return ss.str();
}

std::string ConstantsGenerator::f90_source() const {
  std::ostringstream ss;
  ss << "! This file was generated by EKAT with generate_constants.hpp and \n"
     << "! generate_constants.cpp using data defined in \n"
     << "!        ekat/src/ekat/physics/constants.yaml\n"
     << "!\n"
     << "!        DO NOT EDIT\n"
     << "!\n"
     << "!\n\n";
  ss << "module ekat_base_constants\n"
     << "  use iso_c_binding, only: c_double\n"
     << "  implicit none\n"
     << "  public\n\n";
  ss << "  integer, parameter :: c_real = c_double\n\n";
  for (const auto& pc : base_consts) {
    ss << "! " << pc.second.name << "\n"
       << "!\t units: " << pc.second.units << "\n"
       << "!\t source info:\n";
    for (const auto& s : pc.second.src_info) {
      ss << "!\t\t " << s << "\n\n";
    }
    ss << "real(c_real), parameter :: " << pc.second.software_name << " = ";
    ss << std::setprecision(std::numeric_limits<ConstantsRealType>::digits10 + 1);
    ss << pc.second.value << "_c_real\n";
  }
  ss << "end module\n";
  return ss.str();
}

} // namespace physics
} // namespace ekat
