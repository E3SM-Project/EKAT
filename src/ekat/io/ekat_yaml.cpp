#include "ekat/util/ekat_string_utils.hpp"
#include "ekat/util/ekat_meta_utils.hpp"
#include "ekat/io/ekat_yaml.hpp"
#include "ekat/ekat_assert.hpp"

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <fstream>
#include <iomanip>

namespace ekat {

// These are the values we can parse.
using values_t     = TypeList<bool,int,double,std::string>;

// These are the vector<T> values we can store. char is to store bool.
using seq_values_t = TypeList<char,int,double,std::string>;

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
  std::istringstream is(s);
  int d;
  is >> d;
  return !is.fail() && is.eof();
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

  TypeListFor<values_t>([&](auto t) -> bool{
    using vtype = decltype(t);
    if (is_type<vtype>(str)) {
      list.set(key,str2type<vtype>(str));
      return true;
    }
    return false;
  });
}

template<>
void parse_node<YAML::NodeType::Sequence> (
    const YAML::Node& node,
    const std::string& key,
    ParameterList& list)
{
  EKAT_REQUIRE_MSG (node.Type()==YAML::NodeType::Sequence,
                      "Error! Actual node type incompatible with template parameter.\n");

  // The following fancy typemap/typelist code defeats the nvcc compiler(s)
  // available on Summit, so we're backpedaling for now. The code below this
  // block comment does the same thing. -JNJ, 5/27/2022
  /*
  // Loop over the value types, just to find the one corresponding to seq_type
  using val_to_seq_val = TypeMap<values_t,seq_values_t>;
  int n = node.size();
  TypeListFor<values_t>([&](auto t)->bool {
    using vtype = decltype(t);
    if (is_seq<vtype>(node)) {
      // Once we know the value type, create the proper std::vector, fill it, and set it in the list
      using seq_val_t = val_to_seq_val::at_t<vtype>;
      std::vector<seq_val_t> vec(n);
      for (int i=0; i<n; ++i) {
        std::string str = node[i].as<std::string>();
        vec[i] = str2type<seq_val_t>(str);
      }
      list.set(key,vec);
      // Returning true allows to immediately break from the TypeListFor
      return true;
    }
    return false;
  });
  */
  // Yes, I know. The C preprocessor! Too bad the fancy C++ template stuff
  // isn't up to the task. Here we encode a value type and its related sequence
  // type in the first two arguments in this macro, which we then call on all
  // types in the typelist.
#define TRY_TYPE_ON_NODE(vtype, seq_val_t, node) \
  if (is_seq<vtype>(node)) { \
    int n = node.size(); \
    std::vector<seq_val_t> vec(n); \
    for (int i=0; i<n; ++i) { \
      std::string str = node[i].as<std::string>(); \
        vec[i] = str2type<seq_val_t>(str); \
    } \
    list.set(key,vec); \
    return; \
  }
  TRY_TYPE_ON_NODE(bool, char, node);
  TRY_TYPE_ON_NODE(int, int, node);
  TRY_TYPE_ON_NODE(double, double, node);
  TRY_TYPE_ON_NODE(std::string, std::string, node);
#undef TRY_TYPE_ON_NODE
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
std::string write_param<double> (const double& t) {
  std::stringstream ss;
  ss << std::showpoint << std::setprecision(14) << t;
  std::string s = ss.str();
  auto dot = s.find('.');
  auto last_nz = s.find_last_not_of('0');
  if (last_nz==dot) {
    return s.substr(0,dot+2);
  } else {
    return s.substr(0,last_nz+1);
  }
}
template<>
std::string write_param<char> (const char& t) {
  return (t ? "true" : "false");
}

void write_parameter_list (const ParameterList& params, std::ostream& out, int indent) {
  std::string tab(indent,' ');

  // Helper lambda, to try all possible value types
  auto try_values = [&](const std::string& pname) -> bool {
    bool found = false;
    TypeListFor<values_t>([&](auto t) ->bool {
      using vtype = decltype(t);
      if (params.isType<vtype>(pname)) {
        auto v = params.get<vtype>(pname);
        out << write_param(v) << '\n';
        found = true;
      }
      return found;
    });
    return found;
  };

  // Helper lambda, to try all possible sequence types
  auto try_sequences = [&](const std::string& pname) {
    bool found = false;
    TypeListFor<seq_values_t>([&](auto t) ->bool {
      using vtype = std::vector<decltype(t)>;
      if (params.isType<vtype>(pname)) {
        auto v = params.get<vtype>(pname);
        std::stringstream s;
        for (const auto& it : v)
          s << write_param(it) << ", ";
        // Skip trailing ", " in s
        out << "[" << s.str().substr(0,s.str().size()-2) << "]\n";
        found = true;
      }
      return found;
    });
    return found;
  };

  // Write parameters
  for (auto it=params.params_names_cbegin(); it!=params.params_names_cend(); ++it) {
    const auto& pname = *it;
    out << tab << pname << ": ";
    EKAT_REQUIRE_MSG (try_values(pname) or try_sequences(pname),
          "[write_yaml_file] Error! The writer function can only write the following types:\n\n"
          "  bool, int, double, std::string, \n"
          "  std::vector<char>, std::vector<int>, std::vector<double>, std::vector<std::string>\n\n"
          "where std::vector<char> is interpreted as an array of bool's.\n");
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

