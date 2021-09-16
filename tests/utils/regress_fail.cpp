#include <catch2/catch.hpp>

#include "ekat/ekat_assert.hpp"

namespace {

TEST_CASE ("doomed") {
  REQUIRE(false);
}

} // anonymous namespace
