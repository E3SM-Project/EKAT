#include "catch2/catch.hpp"

#include "ekat/ekat_pack_utils.hpp"

namespace {

TEST_CASE("PackInfo") {

  SECTION ("pack1") {
    using Info = ekat::PackInfo<1>;

    REQUIRE (Info::num_packs(3)==3);

    REQUIRE (!Info::has_padding(1));

    REQUIRE (Info::pack_idx(4)==4);
    REQUIRE (Info::vec_idx(4)==0);

    REQUIRE (Info::pack_vec_idx(4)==Kokkos::make_pair(4,0));

    REQUIRE (Info::array_idx(Kokkos::make_pair(4,0))==4);
  }

  SECTION ("pack8") {
    using Info = ekat::PackInfo<8>;

    REQUIRE (Info::num_packs(16)==2);
    REQUIRE (Info::num_packs(17)==3);

    REQUIRE (!Info::has_padding(16));
    REQUIRE (Info::has_padding(13));


    REQUIRE (Info::pack_idx(7)==0);
    REQUIRE (Info::pack_idx(8)==1);
    REQUIRE (Info::vec_idx(7)==7);
    REQUIRE (Info::vec_idx(8)==0);

    REQUIRE (Info::pack_vec_idx(7)==Kokkos::make_pair(0,7));
    REQUIRE (Info::pack_vec_idx(8)==Kokkos::make_pair(1,0));

    REQUIRE (Info::array_idx(Kokkos::make_pair(1,1))==9);
  }
}

} // namespace
