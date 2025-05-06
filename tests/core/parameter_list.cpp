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
    auto it = lines.end()-6;
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

  auto sl_begin = src.sublists_names_cbegin();
  auto sl_end   = src.sublists_names_cend();
  REQUIRE (std::next(sl_begin,1)==sl_end); // Only one sublist

  auto p_begin = src.params_names_cbegin();
  auto p_end   = src.params_names_cend();
  REQUIRE (std::next(p_begin,2)==p_end); // Two params
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

} // empty namespace
