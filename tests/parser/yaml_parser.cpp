#include <catch2/catch.hpp>

#include "ekat_yaml.hpp"
#include "ekat_string_utils.hpp"

namespace {

template<typename T>
void check_same_param (const ekat::ParameterList& p1,const ekat::ParameterList& p2, const std::string& name)
{
  if ( p1.isType<T>(name) ) {
    if (not p2.isType<T>(name)) {
      std::cout << "p2.isType<int>(" << name << "): " << p2.isType<int>(name) << "\n";
      std::cout << "p2.isType<bool>(" << name << "): " << p2.isType<bool>(name) << "\n";
      std::cout << "p2.isType<double>(" << name << "): " << p2.isType<double>(name) << "\n";
      std::cout << "p2.isType<std::string>(" << name << "): " << p2.isType<std::string>(name) << "\n";
      std::cout << "p2.isType<std::vector<int>>(" << name << "): " << p2.isType<std::vector<int>>(name) << "\n";
      std::cout << "p2.isType<std::vector<char>>(" << name << "): " << p2.isType<std::vector<char>>(name) << "\n";
      std::cout << "p2.isType<std::vector<double>>(" << name << "): " << p2.isType<std::vector<double>>(name) << "\n";
      std::cout << "p2.isType<std::vector<std::string>>(" << name << "): " << p2.isType<std::vector<std::string>>(name) << "\n";
    }
    REQUIRE (p2.isType<T>(name));
    REQUIRE (p1.get<T>(name)==p2.get<T>(name));
  }
}

void check_superset (const ekat::ParameterList& super, const ekat::ParameterList& sub)
{
  for (const auto& sname : super.sublist_names()) {
    REQUIRE (sub.isSublist(sname));

    check_superset(super.sublist(sname),sub.sublist(sname));
  }

  for (const auto& pname : super.param_names()) {
    REQUIRE (sub.isParameter(pname));

    // We can't check EVERY possible type, but we can check the ones we use in this test
    // Check that, if param has type T in super, it also has type T in sub, and values match
    check_same_param<int>(super,sub,pname);
    check_same_param<bool>(super,sub,pname);
    check_same_param<double>(super,sub,pname);
    check_same_param<std::string>(super,sub,pname);
    check_same_param<std::vector<int>>(super,sub,pname);
    check_same_param<std::vector<bool>>(super,sub,pname);
    check_same_param<std::vector<double>>(super,sub,pname);
    check_same_param<std::vector<std::string>>(super,sub,pname);
  }
}

TEST_CASE ("yaml_parser","") {
  using namespace ekat;
  using empty_t = ParameterList::EmptySeq;

  std::string fname = "input.yaml";
  ParameterList params("parameters");
  parse_yaml_file(fname,params);

  REQUIRE (params.isParameter ("ints"));
  REQUIRE (params.isType<std::vector<int>> ("ints"));
  REQUIRE (params.isParameter ("empty_list"));
  REQUIRE (params.isType<empty_t> ("empty_list"));
  REQUIRE (params.get<std::vector<double>> ("empty_list").size()==0);

  // Check some of the loaded parameters.
  // NOTE: if you change input.yaml, you may have to
  //       change some of these checks as well.

  REQUIRE (params.isSublist("Constants"));
  REQUIRE (params.isSublist("Options"));

  auto& constants = params.sublist("Constants");
  auto& options   = params.sublist("Options");

  REQUIRE (constants.isParameter("Two Logicals"));
  REQUIRE (constants.isParameter("Three Doubles"));
  REQUIRE (constants.isParameter("One String"));
  REQUIRE (options.isParameter("My Int"));
  REQUIRE (options.isParameter("My Bool"));

  std::vector<char> logicals;
  REQUIRE_NOTHROW(logicals = constants.get<std::vector<char>>("Two Logicals"));

  REQUIRE (logicals.size()==2);
  REQUIRE (logicals[0] == 1);
  REQUIRE (logicals[1] == 0);

  std::vector<double> doubles;
  REQUIRE_NOTHROW(doubles = constants.get<std::vector<double>>("Three Doubles"));

  std::string str = "multiple words string";
  REQUIRE (constants.get<std::string>("One String") == str);

  REQUIRE (options.get<int>("My Int") == -2);
  REQUIRE (options.get<bool>("My Bool") == false);

  REQUIRE (constants.isSublist("Nested Sublist"));
  auto& sl = constants.sublist("Nested Sublist");
  REQUIRE (sl.get<int>("The Answer") == 42);

  REQUIRE (options.isType<bool>("My Bool"));
  REQUIRE (options.isType<int>("My Int"));
  REQUIRE (options.isType<std::string>("My String"));
  REQUIRE (options.isType<double>("My Real"));
  REQUIRE (!options.isType<bool>("My Real"));
  REQUIRE (!options.isType<int>("My Real"));
  REQUIRE (!options.isType<std::string>("My Real"));
  REQUIRE_THROWS(options.isType<double>("I am not in the list"));

}

TEST_CASE ("write_yaml") {
  using namespace ekat;
  std::string ifile = "input.yaml";
  std::string ofile = "output.yaml";

  ParameterList params1("parameters"),params2("parameters");

  parse_yaml_file(ifile,params1);
  write_yaml_file (ofile,params1);
  parse_yaml_file(ofile,params2);

  // Ensure params in params1 are in params2 and viceversa
  check_superset (params1,params2);
  check_superset (params2,params1);
}

} // anonymous namespace
