#include <catch2/catch.hpp>

#include <cassert>

void my_func () {
  assert (false);
}

TEST_CASE ("assert") {
  EKAT_EXPECT_ASSERT_FAIL (assert (0==1));
  EKAT_EXPECT_ASSERT_FAIL (my_func ());
}
