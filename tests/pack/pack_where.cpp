#include "catch2/catch.hpp"

#include "ekat/util/ekat_where.hpp"

template<typename T>
void run_scalar_tests ()
{
  using namespace ekat;

  T v1 = 0;
  T v2 = 0;
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
}

template<typename T, int N>
void run_packed_tests ()
{
  using PT = ekat::Pack<T,N>;

  PT p1;
  for (int i=0; i<N; ++i) {
    p1[i] = i;
  }
  const T tgt = N/2;
  const int half = N/2;

  PT p2 = p1;

  auto small_p = where(p1<tgt,p2);

  REQUIRE (small_p.any()==(N>1));
  REQUIRE (not small_p.all());
  REQUIRE (small_p.none()==(N==1));
  REQUIRE ((small_p.value()==p2).all());
  REQUIRE (small_p.mask()==(p1<tgt));

  small_p = -1;
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==-1);
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==i);
  }

  p2 = p1;
  small_p += 10;
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]+10));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }

  p2 = p1;
  small_p -= 10;
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]-10));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }

  p2 = p1;
  small_p *= 10;
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]*10));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }

  p2 = p1;
  small_p /= 10;
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]/10));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }
}

TEST_CASE("where") {
  run_scalar_tests<double>();
  run_packed_tests<double,1>();
  run_packed_tests<double,16>();
}
