#include <catch2/catch.hpp>

#include "ekat/util/ekat_string_utils.hpp"
#include "ekat/ekat_pack.hpp"
#include "ekat/kokkos/ekat_kokkos_meta.hpp"
#include "ekat/ekat_type_traits.hpp"

#include "ekat_test_config.h"

namespace {

TEST_CASE("precision", "util") {
  CHECK_FALSE(ekat::is_single_precision<double>::value);
  CHECK(ekat::is_single_precision<float>::value);
}

TEST_CASE("type_traits", "") {
  using namespace ekat;
  REQUIRE(std::is_same<ekat::ValueType<double**&>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double*[3]>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double[2][3]>::type,double>::value);

  // Check meta-util to get rank and dynamic rank of a raw MD array
  REQUIRE(ekat::GetRanks<double[2][3]>::rank==2);
  REQUIRE(ekat::GetRanks<double[2][3]>::rank_dynamic==0);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank==3);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank_dynamic==1);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank==4);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank_dynamic==2);
}

// This is just a compilation test.
TEST_CASE("Unmanaged", "ekat::ko") {
  using ekat::Unmanaged;

  {
    typedef Kokkos::View<double*> V;
    V v("v", 10);
    typedef Unmanaged<V> VUm;
    VUm v_um(v);
    static_assert( ! V::traits::memory_traits::is_unmanaged, "Um");
    static_assert(VUm::traits::memory_traits::is_unmanaged, "Um");
  }

  {
    typedef Kokkos::View<ekat::Pack<double, EKAT_TEST_PACK_SIZE>***,
                         Kokkos::LayoutLeft,
                         Kokkos::HostSpace,
                         Kokkos::MemoryTraits<Kokkos::RandomAccess> >
      V;
    V v("v", 2, 3, 4);
    typedef Unmanaged<V> VUm;
    static_assert(VUm::traits::memory_traits::is_random_access, "Um");
    static_assert(VUm::traits::memory_traits::is_unmanaged, "Um");
    VUm v_um(v);
    typedef Unmanaged<VUm> VUmUm;
    static_assert(VUmUm::traits::memory_traits::is_random_access, "Um");
    static_assert(VUmUm::traits::memory_traits::is_unmanaged, "Um");
    static_assert( ! VUmUm::traits::memory_traits::is_atomic, "Um");
    static_assert( ! VUmUm::traits::memory_traits::is_aligned, "Um");
    static_assert( ! VUmUm::traits::memory_traits::is_restrict, "Um");
    VUmUm v_umum(v);
  }

  {
    typedef Kokkos::View<ekat::Pack<int, EKAT_TEST_PACK_SIZE>[10],
                         Kokkos::HostSpace,
                         Kokkos::MemoryTraits<Kokkos::Atomic | Kokkos::Aligned | Kokkos::Restrict> >
      V;
    static_assert( ! V::traits::memory_traits::is_unmanaged, "Um");
    V v("v");
    typedef Unmanaged<V>::const_type CVUm;
    static_assert(CVUm::traits::memory_traits::is_atomic, "Um");
    static_assert(CVUm::traits::memory_traits::is_aligned, "Um");
    static_assert(CVUm::traits::memory_traits::is_restrict, "Um");
    static_assert(CVUm::traits::memory_traits::is_unmanaged, "Um");

    using Kokkos::Impl::ViewMapping;
    static_assert(ViewMapping<CVUm::traits, V::traits, void>::is_assignable,
                  "CVUm <- V");
    static_assert( ! ViewMapping<V::traits, CVUm::traits, void>::is_assignable,
                  "V </- CVUm");
    static_assert(ViewMapping<CVUm::traits, Unmanaged<V>::traits, void>::is_assignable,
                  "CVUm <- VUm");
    static_assert( ! ViewMapping<Unmanaged<V>::traits, CVUm::traits, void>::is_assignable,
                  "VUm </- CVUm");
    CVUm cv_um(v);
  }
}

TEST_CASE("string","string") {
  using namespace ekat;

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

  SECTION ("utility_functions") {
    std::string my_str  = "item 1  ; item2;  item3 ";
    std::string my_list = "item1;item2;item3";

    strip(my_str,' ');
    REQUIRE(my_str==my_list);

    auto items = split(my_list,';');
    REQUIRE(items.size()==3);
    REQUIRE(items[0]=="item1");
    REQUIRE(items[1]=="item2");
    REQUIRE(items[2]=="item3");
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
