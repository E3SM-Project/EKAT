#include "catch2/catch.hpp"

#include "ekat_where.hpp"

TEST_CASE("where")
{
  using namespace ekat;

  double v1 = 0;
  double v2 = 0;
  auto zero_w = where (v1==0,v2);
  auto one_w = where (v1==1,v2);

  // Internals
  REQUIRE (zero_w.mask());
  REQUIRE (zero_w.value()==0);
  REQUIRE (zero_w.any());
  REQUIRE (zero_w.all());
  REQUIRE (not zero_w.none());

  REQUIRE (not one_w.mask());
  REQUIRE (one_w.value()==0);
  REQUIRE (not one_w.any());
  REQUIRE (not one_w.all());
  REQUIRE (one_w.none());

  // assignment op
  zero_w = 1;
  REQUIRE (v2==1);

  // op= calls
  zero_w += 1;
  REQUIRE (v2==2);
  zero_w -= 1;
  REQUIRE (v2==1);
  zero_w *= 2;
  REQUIRE (v2==2);
  zero_w /= 2;
  REQUIRE (v2==1);

  // max/min
  v2 = 3;
  REQUIRE (max(zero_w,2)==3);
  REQUIRE (max(zero_w,4)==4);
  REQUIRE (min(zero_w,2)==2);
  REQUIRE (min(zero_w,4)==3);
}
