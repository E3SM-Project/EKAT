#include <catch2/catch.hpp>

#include "ekat_parameter_list.hpp"
#include "ekat_string_utils.hpp"

namespace {

TEST_CASE("parameter_list", "") {
  using namespace ekat;

  ParameterList src("src");
  src.set<int>("i",8);
  src.set<int>("j",10);
  src.sublist("sl").set<double>("d",1.0);

  bool thrown = false;
  try {
    printf("d = %f\n",src.get<double>("i"));
  } catch (std::exception& e) {
    auto lines = ekat::split(e.what(),"\n");
    std::vector<std::string> expected = {
      "Error! Attempting to access parameter using the wrong type.",
      "   - list name : src",
      "   - param name: i",
      "   - param type: i",
      "   - input type: d'.",
      ""
    };
    auto it = lines.begin();
    for (const auto& s : expected) {
      REQUIRE (s==*it);
      ++it;
    }
    thrown = true;
  }
  // Check that an exception was thrown, so that the previous check was excercised
  REQUIRE (thrown);

  ParameterList dst("dst");
  dst.set<int>("i",10);

  dst.import(src);

  REQUIRE (dst.get<int>("i")==8);

  REQUIRE (dst.isParameter("j"));
  REQUIRE (dst.get<int>("j")==10);

  REQUIRE (dst.isSublist("sl"));
  REQUIRE (dst.sublist("sl").isParameter("d"));
  REQUIRE (dst.sublist("sl").get<double>("d")==1.0);

  auto p_names = src.param_names();
  REQUIRE (p_names.size()==2); // Two params
  REQUIRE (p_names[0]=="i");
  REQUIRE (p_names[1]=="j");

  auto s_names = src.sublist_names();
  REQUIRE (s_names.size()==1); // One sublist
  REQUIRE (s_names[0]=="sl");
}

TEST_CASE ("pl_copy") {
  ekat::ParameterList pl;
  auto& sub1 = pl.sublist("a");
  sub1.set("int",0);

  ekat::ParameterList pl2 = sub1;
  sub1.get<int>("int") = 2;
  REQUIRE (pl2.get<int>("int")==0);
  pl2.set<int>("int",3);
  REQUIRE (sub1.get<int>("int")==2);
}

TEST_CASE ("pl_empty_seq") {
  using ES = ekat::ParameterList::EmptySeq;
  ekat::ParameterList pl;
  pl.set("seq",ES());
  const auto pl2 = pl;

  auto v1 = pl.get<std::vector<char>>("seq");
  REQUIRE (v1.size()==0);
  REQUIRE_THROWS (pl.get<std::vector<int>>("seq")); // Cannot change type anymore

  // On const param lists, the get method returns a ref to a static var
  const auto& v2 = pl2.get<std::vector<int>>("seq");
  const auto& v3 = pl2.get<std::vector<double>>("seq");
  REQUIRE (v2.size()==0);
  REQUIRE (v3.size()==0);
}

} // empty namespace
