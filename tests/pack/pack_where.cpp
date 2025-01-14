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
    p1[i] = 2*(i+1);
  }

  const int half = N/2;
  ekat::Mask<N> m (false);
  for (int i=0; i<half; ++i)
    m.set(i,true);

  PT p2 = p1;
  PT p3 = -p1;

  auto p_masked = where(m,p2);

  REQUIRE ((p_masked.value()==p2).all());
  REQUIRE (p_masked.mask()==m);
  REQUIRE (p_masked.any()==(N>1));
  REQUIRE (not p_masked.all());
  REQUIRE (p_masked.none()==(N==1));

  // Assignment
  p2 = p1;
  p_masked = -1;   // = scalar
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==-1);
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }
  p2 = p1;
  p_masked = p3;   // = pack
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==p3[i]);
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }

  // operator +=
  p2 = p1;
  p_masked += 2;  // += scalar
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]+2));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }
  p2 = p1;
  p_masked += p3;  // += pack
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==0);
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }

  // operator -=
  p2 = p1;
  p_masked -= 2; // -= scalar
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]-2));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }
  p2 = p1;
  p_masked -= p3; // -= pack
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==2*p1[i]); // b/c p3=-p1
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }

  // opeartor *=
  p2 = p1;
  p_masked *= 2; // *= scalar
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]*2));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }
  p2 = p1;
  p_masked *= p3; // *= pack
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(-p1[i]*p1[i]));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }

  // opeartor /=
  p2 = p1;
  p_masked /= 2; // /= scalar
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==(p1[i]/2));
  }
  for (int i=half; i<N; ++i) {
    REQUIRE (p2[i]==p1[i]);
  }
  p2 = p1;
  p_masked /= p3; // /= pack
  for (int i=0; i<N/2; ++i) {
    REQUIRE (p2[i]==-1); // b/c p3=-p1
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
