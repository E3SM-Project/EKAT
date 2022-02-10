#include <catch2/catch_all.hpp>

#include "ekat/ekat_assert.hpp"

namespace {

TEST_CASE ("doomed") {
  REQUIRE(false);
}

} // anonymous namespace
