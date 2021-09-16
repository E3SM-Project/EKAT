#include "ekat/ekat_parse_yaml_file.hpp"
#include "ekat/ekat_assert.hpp"

#include <stdexcept>
#include <yaml-cpp/yaml.h>

#include <iostream>

namespace ekat {

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
    std::vector<int> vec(n);
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

} // namespace ekat

