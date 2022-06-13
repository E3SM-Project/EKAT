#include <catch2/catch.hpp>
#include "ekat/physics/generate_constants.hpp"
#include <iostream>
#include <fstream>

using namespace ekat;
using namespace ekat::physics;
using Catch::Matchers::Contains;

std::vector<std::string> dummy_file_lines() {
  std::vector<std::string> result;
  result.push_back("base_constants:\n");
  result.push_back("  Pi:\n");
  result.push_back("    software_name: pi\n");
  result.push_back("    value: 3.141592653589793238462643383279502884197169399375105820974944592\n");
  result.push_back("    units: \"n/a\"\n");
  result.push_back("    source:\n");
  result.push_back("      name: \"Mathematica 12.0.0\"\n");
  result.push_back("      command: \"N[Pi, 64]\"\n");
  result.push_back("\n");
  result.push_back("  Avogadro:\n");
  result.push_back("    software_name: avogadro\n");
  result.push_back("    value: 6.02214076e23\n");
  result.push_back("    units: \"#/mol\"\n");
  result.push_back("    source:\n");
  result.push_back("      name: \"NIST CODATA 2018\"\n");
  result.push_back("      url: https://physics.nist.gov/cuu/Constants/index.html\n");
  result.push_back("      citation: \"E. Tiesinga, P. J. Mohr, D. B. Newell, and B. N. Taylor, 2021, CODATA recommended values of the fundamental physical constants: 2018, Rev. Modern Phys. 93.\"\n");
  result.push_back("\n");
  result.push_back("  One:\n");
  result.push_back("    software_name: one\n");
  result.push_back("    value: 1\n");
  result.push_back("    units: \"n/a\"\n");
  result.push_back("    source:\n");
  result.push_back("      name: \"1\"\n");
  result.push_back("\n");
  result.push_back("  Two:\n");
  result.push_back("    software_name: two\n");
  result.push_back("    value: 2\n");
  result.push_back("    units: \"n/a\"\n");
  result.push_back("    source:\n");
  result.push_back("      name: \"2\"\n");
  result.push_back("\n");
  result.push_back("derived_constants:\n");
  result.push_back("  Three:\n");
  result.push_back("    depends: [One, Two]\n");
  result.push_back("    software_name: three\n");
  result.push_back("    units: \"n/a\"\n");
  result.push_back("    definition: tow + one\n"); // purposeful mispelling of "two"
  return result;
}

std::string concat_lines(const std::vector<std::string>& lines,
                         const int skip_start=-1,
                         const int skip_len=1) {
  std::string result;
  for (int i=0; i<lines.size(); ++i) {
    if (skip_len > 0) {
      if ( i < skip_start or i >= skip_start + skip_len) {
        result += lines[i];
      }
    }
    else {
      if ( i < skip_start) {
        result += lines[i];
      }
    }
  }
  return result;
}

TEST_CASE("read constants", "") {

  SECTION("dummy file") {
    const std::string dfname = "dummy_consts.yaml";

    const auto flines = dummy_file_lines();

    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 2);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Pi: software_name not found"));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 3);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Pi: value not found"));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 4);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Pi: units not found"));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 5, 3);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Pi: source information not found"));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 32, -1);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("dummy_consts.yaml: No derived_constants section found."));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 34);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Three: derived dependencies not found."));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 35);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Three: software_name not found."));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 36);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Three: units not found."));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 37);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Three: derived definition not found."));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 25, 7);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Three: derived constant dependency Two not found."));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines);
      dum_file.close();

      REQUIRE_THROWS_WITH(ConstantsGenerator(dfname), Contains("Three: derived dependency software_name not found."));
    }
    {
      std::ofstream dum_file(dfname);
      dum_file << concat_lines(flines, 37);
      dum_file << "    definition: two + one\n"; // correct spelling of "two"
      dum_file.close();

      REQUIRE_NOTHROW(ConstantsGenerator(dfname));
    }
  }

  SECTION("main file") {
    std::string dfname = "./constants.yaml";
    ConstantsGenerator gen(dfname);
  }
}
