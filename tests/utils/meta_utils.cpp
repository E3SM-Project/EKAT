#include <catch2/catch.hpp>

#include "ekat/util/ekat_meta_utils.hpp"

// We can't define template alias inside the TEST_CASE block,
// since, upon macro expansion, those blocks are at function scope.

template<typename T>
using TestVec = std::vector<T>;

TEST_CASE ("meta_utils") {
  using namespace ekat;

  using L1 = TypeList<int,float,double>;
  using L2 = TypeList<char,void>;
  using L3 = TypeList<int,char>;

  SECTION ("base") {
    REQUIRE (std::is_same<L1::head,int>::value);
    constexpr int L1size = L1::size;
    REQUIRE (L1size==3);
  }

  SECTION ("cat") {
    using L1L2 = typename CatLists<L1,L2>::type;
    REQUIRE (std::is_same<L1L2,TypeList<int,float,double,char,void>>::value);
    constexpr int L1L2size = L1L2::size;
    REQUIRE (L1L2size==5);
  }

  SECTION ("unique") {
    using L1L2 = typename CatLists<L1,L2>::type;
    using L1L3 = typename CatLists<L1,L3>::type;

    REQUIRE (UniqueTypeList<L1L2>::value);
    REQUIRE (not UniqueTypeList<L1L3>::value);
  }

  SECTION ("first_of") {
    constexpr auto f_pos = FirstOf<float,L1>::pos;
    constexpr auto v_pos = FirstOf<void,L1>::pos;

    REQUIRE (f_pos==1);
    REQUIRE (v_pos==-1);
  }

  SECTION ("access") {
    REQUIRE (std::is_same<typename AccessList<L1,0>::type,int>::value);
    REQUIRE (std::is_same<typename AccessList<L1,1>::type,float>::value);
    REQUIRE (std::is_same<typename AccessList<L1,2>::type,double>::value);
  }

  SECTION ("type_map") {
    using map_t = TypeMap<L2,L3>;

    constexpr auto map_size = map_t::size;
    REQUIRE (map_size==2);

    map_t map;
    auto i = map.at<char>();
    auto c = map.at<void>();
    REQUIRE (std::is_same<decltype(i),int>::value);
    REQUIRE (std::is_same<decltype(c),char>::value);
  }

  SECTION ("apply_template") {
    using Templates = typename ApplyTemplate<TestVec,L1>::type;
    // Templates should be a type list of TestVec<blah> types.
    REQUIRE (std::is_same<typename AccessList<Templates,0>::type,TestVec<int>>::value);
    REQUIRE (std::is_same<typename AccessList<Templates,1>::type,TestVec<float>>::value);
    REQUIRE (std::is_same<typename AccessList<Templates,2>::type,TestVec<double>>::value);
  }

  SECTION ("list_for") {
    using K = L1;
    using V = typename ApplyTemplate<TestVec,K>::type;
    using map_t = TypeMap<K,V>;

    map_t map;
    map.at<int>().resize(2);
    map.at<float>().resize(3);
    map.at<double>().resize(1);

    int size = 0;
    TypeListFor<K>([&](auto t) {
      const auto& m = map.at<decltype(t)>();
      size += m.size();
    });

    REQUIRE (size==6);
  }
}
