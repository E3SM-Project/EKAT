#include <catch2/catch.hpp>

#include "ekat/util/ekat_string_utils.hpp"

namespace {

TEST_CASE("string","string") {
  using namespace ekat;

  SECTION ("string_manipulation") {
    std::string my_str  = "item 1  ; item2;  item3 ";
    std::string my_list = "item1;item2;item3";

    strip(my_str,' ');
    REQUIRE(my_str==my_list);

    auto items = split(my_list,';');
    REQUIRE(items.size()==3);
    REQUIRE(items[0]=="item1");
    REQUIRE(items[1]=="item2");
    REQUIRE(items[2]=="item3");

    std::string padded   = "*****my**string**";
    std::string unpadded = "my**string";
    REQUIRE (trim(padded,'*')==unpadded);

    std::string lower = "my_lower case string!";
    std::string upper = "MY_LOWER CASE STRING!";
    REQUIRE (upper==upper_case(lower));
  }

  SECTION ("case_insensitive_string") {
    CaseInsensitiveString cis1 = "field_1";
    CaseInsensitiveString cis2 = "fIeLd_1";
    CaseInsensitiveString cis3 = "field_2";
    CaseInsensitiveString cis4 = "feld_1";

    REQUIRE (cis1==cis2);
    REQUIRE (cis1!=cis3);
    REQUIRE (cis4<=cis1);
    REQUIRE (cis4<cis1);
  }

  SECTION ("jaro_similarity") {
    // Jaro and Jaro-Winkler similarity tests

    // Benchmark list (including expected similarity values) from Winkler paper
    //  https://www.census.gov/srd/papers/pdf/rrs2006-02.pdf
    // Note: Winkler clamps all values below 0.7 to 0. I don't like that,
    //       so I had to remove some entries.

    //                          LHS         RHS       JARO   JARO-WINKLER
    using entry_type = std::tuple<std::string,std::string,double, double>;

    std::vector<entry_type> benchmark =
    {
      entry_type{ "shackleford", "shackelford", 0.970, 0.982 },
      entry_type{ "dunningham" , "cunnigham"  , 0.896, 0.896 },
      entry_type{ "nichleson"  , "nichulson"  , 0.926, 0.956 },
      entry_type{ "jones"      , "johnson"    , 0.790, 0.832 },
      entry_type{ "massey"     , "massie"     , 0.889, 0.933 },
      entry_type{ "abroms"     , "abrams"     , 0.889, 0.922 },
      entry_type{ "jeraldine"  , "geraldine"  , 0.926, 0.926 },
      entry_type{ "marhta"     , "martha"     , 0.944, 0.961 },
      entry_type{ "michelle"   , "michael"    , 0.869, 0.921 },
      entry_type{ "julies"     , "julius"     , 0.889, 0.933 },
      entry_type{ "tanya"      , "tonya"      , 0.867, 0.880 },
      entry_type{ "dwayne"     , "duane"      , 0.822, 0.840 },
      entry_type{ "sean"       , "susan"      , 0.783, 0.805 },
      entry_type{ "jon"        , "john"       , 0.917, 0.933 },
    };

    const double tol = 0.005;
    for (const auto& entry : benchmark) {
      const auto& s1 = std::get<0>(entry);
      const auto& s2 = std::get<1>(entry);
      double sj  = jaro_similarity(s1,s2);
      double sjw = jaro_winkler_similarity(s1,s2);

      const double sj_ex = std::get<2>(entry);
      const double sjw_ex = std::get<3>(entry);

      REQUIRE (std::fabs(sj-sj_ex)<tol);
      REQUIRE (std::fabs(sjw-sjw_ex)<tol);
    }
  }

  SECTION ("gather_tokens") {
    std::string s = "my_birthday_is coming soon";
    std::vector<char> delims = {'_',' '};

    auto no_atomic = gather_tokens(s,delims);
    auto atomic_1  = gather_tokens(s,delims,"my_birth");
    auto atomic_2  = gather_tokens(s,delims,"my_birthday");

    REQUIRE (no_atomic.size()==5);
    REQUIRE (atomic_1.size()==5);
    REQUIRE (atomic_2.size()==4);
  }

  // Jaccard (token-based) similarity test.
  SECTION ("jaccard_similarity") {
    using entry_type = std::tuple<std::string,std::string,double>;

    std::vector<entry_type> benchmark =
    {
      entry_type{ "hello world", "world_hello", 1.000},
      entry_type{ "hello_new_world", "hello world", 0.6666667},
    };

    const double tol = 0.001;
    for (const auto& entry : benchmark) {
      // We tokenize strings using spaces and underscores.
      const auto& s1 = std::get<0>(entry);
      const auto& s2 = std::get<1>(entry);
      double s12 = jaccard_similarity(s1,s2,{' ', '_'});
      double s21 = jaccard_similarity(s1,s2,{' ', '_'});
      const double s_ex = std::get<2>(entry);

      // Check against expected value
      REQUIRE (std::abs(s12-s_ex)<tol);
      // Check simmetry
      REQUIRE (std::abs(s12-s21)<tol);
    }

    // Check similarity when not tokenizing one of the arguments
    using entry2_type = std::tuple<std::string,std::string,double,double>;

    std::vector<entry2_type> benchmark2 =
    {
      entry2_type{ "air pressure at sea level altitude", "air pressure", 0.333, 0.2},
      entry2_type{ "pressure of water in air", "air pressure", 0.4, 0.0},
    };

    for (const auto& entry : benchmark2) {
      // We tokenize strings using spaces and underscores.
      const auto& s1 = std::get<0>(entry);
      const auto& s2 = std::get<1>(entry);

      double s12_tokenize = jaccard_similarity(s1,s2,{' ', '_'});
      const double s12_tokenize_ex = std::get<2>(entry);

      double s12_no_tokenize = jaccard_similarity(s1,s2,{' ', '_'},true,false);
      const double s12_no_tokenize_ex = std::get<3>(entry);

      REQUIRE (std::abs(s12_tokenize-s12_tokenize_ex)<tol);
      REQUIRE (std::abs(s12_no_tokenize-s12_no_tokenize_ex)<tol);
    }
  }
}

} // empty namespace

