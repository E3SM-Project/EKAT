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

  // max/min (where expression vs scalar comparison)
  v2 = 3;
  REQUIRE (max(zero_w,2)==3);
  REQUIRE (max(zero_w,4)==4);
  REQUIRE (min(zero_w,2)==2);
  REQUIRE (min(zero_w,4)==3);

  // masked max/min return the scalar when mask is false
  REQUIRE (max(one_w,7)==7);
  REQUIRE (min(one_w,7)==7);

  // reduce methods: mask true -> return value; mask false -> return identity
  v2 = 3;
  REQUIRE (zero_w.reduce_sum() == 3);
  REQUIRE (zero_w.reduce_max() == 3);
  REQUIRE (zero_w.reduce_min() == 3);
  REQUIRE (one_w.reduce_sum()  == Kokkos::reduction_identity<double>::sum());

  // lvalue mask stored by reference: changing the mask variable is reflected
  bool mask_var = true;
  auto ref_w = where(mask_var, v2);
  REQUIRE (ref_w.any());
  mask_var = false;
  REQUIRE (not ref_w.any());   // stored by ref, so picks up the change
  mask_var = true;

  // math functions (applied only where mask is true)
  v2 = 4.0;
  REQUIRE (sqrt(zero_w) == 2.0);
  REQUIRE (abs(zero_w)  == 4.0);
  REQUIRE (pow(zero_w, 2.0) == 16.0);
  REQUIRE (exp(zero_w)  == Kokkos::exp(4.0));
  // when mask is false the original value is returned unchanged
  REQUIRE (sqrt(one_w)  == v2);
}
