#include <catch2/catch.hpp>

#include <cassert>

void my_func () {
  assert (false);
}

TEST_CASE ("assert") {
  EKAT_TEST_ASSERT (assert (0==1));
  EKAT_TEST_ASSERT (my_func ());
}
