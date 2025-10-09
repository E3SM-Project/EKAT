#include "catch2/catch.hpp"

#include "ekat_pack.hpp"
#include "ekat_pack_where.hpp"

template<typename T, int N>
void run_tests ()
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

  // max/min
  ekat::Mask<N> even (false);
  int min_at_even = 1, max_at_even = 0;
  T sum_at_even = 0;
  T prod_at_even = 1;
  for (int i=0; i<N; ++i) {
    p2[i] = i+1;
    if (i % 2 == 0) {
      even.set(i,true);
      max_at_even = std::max(max_at_even,i+1);
      sum_at_even += p2[i];
      prod_at_even *= p2[i];
    }
  }

  auto p_at_even = where(even,p2);

  REQUIRE (max(p_at_even,N)==N);
  REQUIRE (max(p_at_even,-1)==max_at_even);
  REQUIRE (min(p_at_even,-1)==-1);
  REQUIRE (min(p_at_even,N)==min_at_even);

  // reductions
  T sum = 0, prod = 1;
  sum += p_at_even;
  prod *= p_at_even;
  REQUIRE (sum==sum_at_even);
  REQUIRE (prod==prod_at_even);
}

TEST_CASE("where") {
  run_tests<double,1>();
  run_tests<double,16>();
}
