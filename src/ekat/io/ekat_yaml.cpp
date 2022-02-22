#include "ekat/io/ekat_yaml.hpp"
#include "ekat/ekat_assert.hpp"

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <fstream>

namespace ekat {

// =============================== READ ============================ //

template<YAML::NodeType::value Type>
void parse_node (const YAML::Node& node,
                 const std::string& key,
                 ParameterList& list);

// Helpers
bool is_bool (const std::string& s) {
  return s=="true" || s=="false" ||
         s=="TRUE" || s=="FALSE";
}

bool str2bool (const std::string& s) {
  return (s=="true" || s=="TRUE");
}

bool is_int (const std::string& s) {
  std::istringstream is(s);
  int d;
  is >> d;
  return !is.fail() && is.eof();
}

bool is_double (const std::string& s) {
  std::istringstream is(s);
  double d;
  is >> d;
  return !is.fail() && is.eof();
}

// ---------- IMPLEMENTATION -------------- // 

template<>
void parse_node<YAML::NodeType::Scalar> (
    const YAML::Node& node,
    const std::string& key,
    ParameterList& list)
{
  EKAT_REQUIRE_MSG (node.Type()==YAML::NodeType::Scalar,
                      "Error! Actual node type incompatible with template parameter.\n");

  // Extract scalar as string, then try some casts
  // First int, then double, and finally fall back to string
  std::string str = node.as<std::string>();
  if (is_int(str)) {
    list.set(key,std::stoi(str));
  } else if (is_bool(str)) {
    list.set(key,str2bool(str));
  } else if (is_double(str)) {
    list.set(key,std::stod(str));
  } else {
    list.set(key,str);
  }
}

template<>
void parse_node<YAML::NodeType::Sequence> (
    const YAML::Node& node,
    const std::string& key,
    ParameterList& list)
{
  EKAT_REQUIRE_MSG (node.Type()==YAML::NodeType::Sequence,
                      "Error! Actual node type incompatible with template parameter.\n");

  constexpr int str_type = 0;
  constexpr int dbl_type = 1;
  constexpr int int_type = 2;
  constexpr int bool_type = 3;
  int seq_type = -1;
  int n = node.size();
  for (int i=0; i<n; ++i) {
    std::string str = node[i].as<std::string>();
    int ith_type = is_int(str)
                     ? int_type
                     : (is_double(str)
                          ? dbl_type
                          : (is_bool(str) ? bool_type : str_type));

    EKAT_REQUIRE_MSG(seq_type==-1 || seq_type==ith_type,
                       "Error! Found a squence with entries of mixed types.\n"
                       "       Common case is a sequence with int and doubles, such as\n"
                       "          [ 2.3 1 ]\n");

    seq_type = ith_type;
  }

  if (seq_type==int_type) {
    std::vector<int> vec(n);
    for (int i=0; i<n; ++i) {
      std::string str = node[i].as<std::string>();
      vec[i] = std::stoi(str);
    }
    list.set(key,vec);
  } else if (seq_type==bool_type) {
    // NOTE: std::vector<bool> is a BAD thing to use.
    std::vector<char> vec(n);
    for (int i=0; i<n; ++i) {
      std::string str = node[i].as<std::string>();
      vec[i] = str2bool(str) ? 1 : 0;
    }
    list.set(key,vec);
  } else if (seq_type==dbl_type) {
    std::vector<double> vec(n);
    for (int i=0; i<n; ++i) {
      std::string str = node[i].as<std::string>();
      vec[i] = std::stod(str);
    }
    list.set(key,vec);
  } else {
    std::vector<std::string> vec(n);
    for (int i=0; i<n; ++i) {
      vec[i] = node[i].as<std::string>();
    }
    list.set(key,vec);
  }
}

template<>
void parse_node<YAML::NodeType::Map> (
    const YAML::Node& node,
    const std::string& key,
    ParameterList& list)
{
  using YNT = YAML::NodeType;
  EKAT_REQUIRE_MSG (node.Type()==YNT::Map,
                      "Error! Actual node type incompatible with template parameter.\n");

  ParameterList& sublist = list.sublist(key);
  for (auto it : node) {
    std::string item_key = it.first.as<std::string>();
    const auto& item = it.second;

    switch (it.second.Type()) {
      case YNT::Null:
        printf("Null node\n");
        break;
      case YNT::Scalar:
        parse_node<YNT::Scalar>(item, item_key, sublist);
        break;
      case YNT::Undefined:
        printf("Undefined node\n");
        break;
      case YNT::Map:
        parse_node<YNT::Map>(item, item_key, sublist);
        break ;
      case YAML::NodeType::Sequence:
        parse_node<YNT::Sequence>(item, item_key, sublist);
        break;
      default:
        printf("Unexpected node type\n");
    }
  }
}

ParameterList parse_yaml_file (const std::string& fname) {
  ParameterList params;
  parse_yaml_file(fname,params);
  return params;
}

void parse_yaml_file (const std::string& fname, ParameterList& params) {
  YAML::Node root;
  try {
    root = YAML::LoadFile(fname);
  } catch (YAML::BadFile&) {
    EKAT_ERROR_MSG ("Error! Something went wrong while opening file " + fname + "'.\n");
  }
  ParameterList temp(params.name());
  parse_node<YAML::NodeType::Map> (root, temp.name(), temp);
  params = temp.sublist(params.name());
}

// =============================== WRITE ============================ //

void write_parameter_list (const ParameterList& params, std::ostream& out, int indent) {
  std::string tab(indent,' ');

  // Write parameters
  for (auto it=params.params_names_cbegin(); it!=params.params_names_cend(); ++it) {
    const auto& pname = *it;
    out << tab << pname << ": ";
    if (params.isType<bool>(pname)) {
      auto b = params.get<bool>(pname);
      out << (b ? "true" : "false");
    } else if (params.isType<int>(pname)) {
      auto i = params.get<int>(pname);
      out << i;
    } else if (params.isType<double>(pname)) {
      auto d = params.get<double>(pname);
      out << std::showpoint << d;
    } else if (params.isType<std::string>(pname)) {
      auto s = params.get<std::string>(pname);
      out << s;
    } else if (params.isType<std::vector<char>>(pname)) {
      auto cv = params.get<std::vector<char>>(pname);
      int n = cv.size();
      if (n==0) {
        out << "[]";
      } else {
        out << '[' << (cv[0] ? "true" : "false");
        for (int i=1; i<n; ++i) {
          out << ", " << (cv[i] ? "true" : "false");
        }
        out << ']';
      }
    } else if (params.isType<std::vector<int>>(pname)) {
      auto sv = params.get<std::vector<int>>(pname);
      int n = sv.size();
      if (n==0) {
        out << "[]";
      } else {
        out << '[' << sv[0];
        for (int i=1; i<n; ++i) {
          out << ", " << sv[i];
        }
        out << ']';
      }
    } else if (params.isType<std::vector<double>>(pname)) {
      auto sv = params.get<std::vector<double>>(pname);
      int n = sv.size();
      if (n==0) {
        out << "[]";
      } else {
        out << '[' << sv[0];
        for (int i=1; i<n; ++i) {
          out << ", " << sv[i];
        }
        out << ']';
      }
    } else if (params.isType<std::vector<std::string>>(pname)) {
      auto sv = params.get<std::vector<std::string>>(pname);
      int n = sv.size();
      if (n==0) {
        out << "[]";
      } else {
        out << '[' << sv[0];
        for (int i=1; i<n; ++i) {
          out << ", " << sv[i];
        }
        out << ']';
      }
    } else {
      EKAT_ERROR_MSG (
          "[write_yaml_file] Error! The writer function can only write the following types:\n\n"
          "  bool, int, double, std::string, \n"
          "  std::vector<char>, std::vector<int>, std::vector<double>, std::vector<std::string>\n\n"
          "where std::vector<char> is interpreted as an array of bool's.\n");
    }
    
    out << "\n";
  }

  // Write sublists
  for (auto it=params.sublists_names_cbegin(); it!=params.sublists_names_cend(); ++it) {
    out << tab << *it << ":\n";
    write_parameter_list(params.sublist(*it),out,indent+2);
  }
}

void write_yaml_file (const std::string& fname, const ParameterList& params) {
  // YAML::Emitter emitter;

  // YAML::Node root = parameter_list_to_yaml_node(params);
  std::ofstream ofile(fname);

  // Header
  ofile << "%YAML 1.1\n"
        << "---\n";

  // Body
  write_parameter_list (params,ofile,0);

  // Footer
  ofile << "...\n";
}

} // namespace ekat

