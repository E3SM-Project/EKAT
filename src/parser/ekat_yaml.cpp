#include "ekat_yaml.hpp"
#include "ekat_string_utils.hpp"
#include "ekat_assert.hpp"

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <fstream>
#include <iomanip>

namespace ekat {

// =============================== READ ============================ //

template<YAML::NodeType::value Type>
void parse_node (const YAML::Node& node,
                 const std::string& key,
                 ParameterList& list);

// Check if input string can be interpreted as given type
template<typename T>
bool is_type (const std::string& s);

template<>
bool is_type<bool> (const std::string& s) {
  return s==CaseInsensitiveString("true") ||
         s==CaseInsensitiveString("false");
}
template<>
bool is_type<int> (const std::string& s) {
  if (s.size()==0) return false;
  if (s[0]=='-') {
    return is_type<int>(s.substr(1));
  }

  return s.find_first_not_of("0123456789")==std::string::npos;
}

template<>
bool is_type<double> (const std::string& s) {
  std::istringstream is(s);
  double d;
  is >> d;
  return !is.fail() && is.eof();
}
template<>
bool is_type<std::string> (const std::string&) {
  // This is the fallback case. A string...is a string.
  return true;
}

// Convert a string to the given type
template<typename T>
T str2type (const std::string& s);

template<>
bool str2type<bool> (const std::string& s) {
  return s==CaseInsensitiveString("true");
}
template<>
char str2type<char> (const std::string& s) {
  return (s=="true" || s=="TRUE") ? 1 : 0;
}
template<>
int str2type<int> (const std::string& s) {
  return std::stoi(s);
}
template<>
double str2type<double> (const std::string& s) {
  return std::stod(s);
}
template<>
std::string str2type<std::string> (const std::string& s) {
  return s;
}

// Returns true if all entries of the node sequence can be
// interpreted as values of type T
template<typename T>
bool is_seq (const YAML::Node& node) {
  int n = node.size();
  for (int i=0; i<n; ++i) {
    std::string str = node[i].as<std::string>();
    if (not is_type<T>(str)) {
      return false;
    }
  }
  return true;
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
  const auto& tag = node.Tag();

  if (tag=="?") {
    // There's no tag annotation regarding the node type,
    // so we just try in order bool, int, double, string
    if (is_type<bool>(str)) {
      list.set(key,str2type<bool>(str));
    } else if (is_type<int>(str)) {
      list.set(key,str2type<int>(str));
    } else if (is_type<double>(str)) {
      list.set(key,str2type<double>(str));
    } else {
      // Nothing worked, so just use string type
      list.set(key,str);
    }
  } else {
    // YAY, the user is telling us how to interpret the values
    if (tag=="!!bool" or tag=="tag:yaml.org,2002:bool") {
      EKAT_REQUIRE_MSG (is_type<bool>(str),
          "Error! Tag " + tag + " not compatible with the stored value '" + str + "'\n");
      list.set(key,str2type<bool>(str));
    } else if (tag=="!!int" or tag=="tag:yaml.org,2002:int") {
      EKAT_REQUIRE_MSG (is_type<int>(str),
          "Error! Tag " + tag + " not compatible with the stored value '" + str + "'\n");
      list.set(key,str2type<int>(str));
    } else if (tag=="!!float" or tag=="tag:yaml.org,2002:float") {
      EKAT_REQUIRE_MSG (is_type<double>(str),
          "Error! Tag " + tag + " not compatible with the stored value '" + str + "'\n");
      list.set(key,str2type<double>(str));
    } else if (tag=="!" or tag=="!!str" or tag=="tag:yaml.org,2002:str") {
      list.set(key,str);
    } else {
      EKAT_ERROR_MSG ("Error! Unrecognized/unsupported node tag '" + tag + "' for scalar node '" + key + "'.\n"
          "  Supported tags: !!int, !!bool, !!float, !!str");
    }
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

  // For empty sequences, use the parameter list placeholder EmptySeq
  if (node.size()==0) {
    list.set(key,ekat::ParameterList::EmptySeq());
    return;
  }

  auto set_vec = [&](auto t) {
    using seq_val_t = decltype(t);
    int n = node.size();
    std::vector<seq_val_t> vec(n);
    for (int i=0; i<n; ++i) {
      std::string str = node[i].as<std::string>();
      vec[i] = str2type<seq_val_t>(str);
    }
    list.set(key,vec);
  };

  // There's no tag annotation to specify what the list entries are.
  // So we just try in order bool, int, double, string. We choose this
  // order as string would recognize any entry (everything is a string),
  // and double would accept integer inputs too
  if (is_seq<bool>(node)) {
    set_vec('a');
  } else if (is_seq<int>(node)) {
    set_vec(0);
  } else if (is_seq<double>(node)) {
    set_vec(0.0);
  } else {
    set_vec(std::string(""));
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

    switch (item.Type()) {
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

// Helper functions to allow printing values correctly. In particular:
//  - char(0) and char(1) printed as "false", "true"
//  - double printed with at least 1 decimal digit, and no trailing zeros

template<typename T>
std::string write_param (const T& t) {
  std::stringstream s;
  s << std::boolalpha << t;
  return s.str();
}

template<>
std::string write_param<double> (const double& d) {
  std::stringstream ss;
  ss << std::showpoint << std::setprecision(14) << d;
  if (d==static_cast<int>(d) and ss.str().find('.')==std::string::npos) {
    ss << ".0";
  }

  // Clip trailing 0's, but ensure that there is AT LEAST one decimal digit
  auto s = ss.str();
  auto start = s.find('.');
  auto last_nnz = s.substr(start+2).find_last_not_of('0');
  if (last_nnz==std::string::npos) {
    // No nonzeros after the 1st decimal digit. Truncate
    return s.substr(0,start+2);
  }
  return s.substr(0,start+2+last_nnz+1);
}
template<>
std::string write_param<char> (const char& t) {
  return (t ? "true" : "false");
}

void write_parameter_list (const ParameterList& params, YAML::Emitter& out) {
  // Helper lambda, to try all possible value types, return whether we found the type
  auto try_values = [&](const std::string& pname) -> bool {
    if (params.isType<int>(pname)) {
      auto v = params.get<int>(pname);
      out << YAML::Key << pname << YAML::Value << v;
    } else if (params.isType<bool>(pname)) {
      auto v = params.get<bool>(pname);
      out << YAML::Key << pname << YAML::Value << v;
    } else if (params.isType<double>(pname)) {
      auto v = params.get<double>(pname);
      out << YAML::Key << pname << YAML::Value << write_param(v);
    } else if (params.isType<std::string>(pname)) {
      auto v = params.get<std::string>(pname);
      out << YAML::Key << pname << YAML::Value;
      if (v.find('\n') != std::string::npos)
        out << YAML::Literal; // Use Literal for multiline
      out << v;
    } else {
      // No match
      return false;
    }
    return true;
  };

  // Helper lambda, to try all possible sequence types, return whether we found the type
  auto try_sequences = [&](const std::string& pname) {
    std::vector<int> ivec;
    std::vector<char> cvec;
    std::vector<double> dvec;
    std::vector<std::string> svec;
    std::string tag;
    if (params.isType<std::vector<int>>(pname)) {
      ivec = params.get<std::vector<int>>(pname);
      tag = "ints";
    } else if (params.isType<std::vector<char>>(pname)) {
      cvec = params.get<std::vector<char>>(pname);
      tag = "bools";
    } else if (params.isType<std::vector<double>>(pname)) {
      dvec = params.get<std::vector<double>>(pname);
      tag = "floats";
    } else if (params.isType<std::vector<std::string>>(pname)) {
      svec = params.get<std::vector<std::string>>(pname);
      tag = "strings";
    } else {
      // No match
      return false;
    }

    // Note: all but one of the vectors above will be empty, so write will do nothing for all but one
    auto write = [&](auto vec) {
      for (auto e : vec) {
        out << write_param(e);
      }
    };

    auto len = [&](auto vec) {
      int ret = 0;
      for (auto e : vec) {
        ret += write_param(e).size();
      }
      ret += 2*(vec.size()-1); // due to ", " between items
      return ret;
    };

    int seq_len = len(ivec)+len(cvec)+len(dvec)+len(svec);

    out << YAML::Key << pname << YAML::Value << YAML::_Tag("",tag,YAML::_Tag::Type::NamedHandle);

    // Format sequences inline only if reasonably short
    if (seq_len<100)
      out << YAML::Flow;
    else
      out << YAML::Block;
    
    out << YAML::BeginSeq;
    write(ivec);
    write(cvec);
    write(dvec);
    write(svec);
    out << YAML::EndSeq;
    return true;
  };

  // Write parameters
  for (const auto& pname : params.param_names()) {
    EKAT_REQUIRE_MSG (try_values(pname) or try_sequences(pname),
          "[write_yaml_file] Error! The writer function can only write the following types:\n\n"
          "  bool, int, double, std::string, \n"
          "  std::vector<char>, std::vector<int>, std::vector<double>, std::vector<std::string>\n\n"
          "where std::vector<char> is interpreted as an array of bool's.\n");
  }

  // Write sublists
  for (const auto& sname : params.sublist_names()) {
    out << YAML::Key << sname << YAML::Value << YAML::BeginMap;
    write_parameter_list(params.sublist(sname),out);
    out << YAML::EndMap;
  }
}

void write_yaml_file (const std::string& fname, const ParameterList& params) {
  YAML::Emitter out;

  // Start the YAML document
  out << YAML::BeginMap;

  // Write the parameter list
  write_parameter_list (params,out);

  // End the document
  out << YAML::EndMap;

  // Write to file
  std::ofstream ofile(fname);
  EKAT_REQUIRE_MSG (ofile.is_open(),
      "Error! Could not open file for writing.\n"
      " - file name: " + fname + "\n");

  ofile << "%YAML 1.2\n"
        << "---\n"
        << out.c_str() << "\n"
        << "...\n";
}

} // namespace ekat

