#include <catch2/catch.hpp>

#include "ekat/io/ekat_yaml.hpp"
#include "ekat/util/ekat_string_utils.hpp"

#include <fstream>

namespace {

TEST_CASE ("yaml_parser","") {
  using namespace ekat;

  std::string fname = "input.yaml";
  ParameterList params("parameters");
  REQUIRE_NOTHROW ( parse_yaml_file(fname,params) );

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

  // The two files might not match, since the order of yaml entries
  // might not be preserved. So print param lists, split by line,
  // and check that they have the same lines.
  std::stringstream ss1,ss2;
  params1.print(ss1);
  params2.print(ss2);

  auto lines1 = split(ss1.str(),'\n');
  auto lines2 = split(ss2.str(),'\n');

  REQUIRE (lines1.size()==lines2.size());
  for (auto line : lines1) {
    REQUIRE (contains(lines2,line));
  }
}

} // anonymous namespace
