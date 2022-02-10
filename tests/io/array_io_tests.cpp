#include <catch2/catch_all.hpp>

#include "ekat/ekat_pack.hpp"

extern "C"{
int test_array_io ();
}

namespace {

TEST_CASE("array_io_mod", "test_array_io") {

  int nerr = test_array_io();

  REQUIRE (nerr==0);
}

} // empty namespace
