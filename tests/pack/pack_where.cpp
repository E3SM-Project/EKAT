#include "catch2/catch.hpp"

#include "ekat_pack.hpp"
#include "ekat_pack_where.hpp"

#include <cmath>

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

  // --- max/min and reductions over even positions ---
  ekat::Mask<N> even (false);
  T max_at_even = T(0);
  T min_at_even = T(N+1);
  T sum_at_even = T(0);
  for (int i=0; i<N; ++i) {
    p2[i] = i+1;
    if (i % 2 == 0) {
      even.set(i,true);
      max_at_even = std::max(max_at_even, T(i+1));
      min_at_even = std::min(min_at_even, T(i+1));
      sum_at_even += p2[i];
    }
  }

  auto p_at_even = where(even, p2);

  // reduce methods
  REQUIRE (p_at_even.reduce_sum() == sum_at_even);
  REQUIRE (p_at_even.reduce_max() == max_at_even);
  REQUIRE (p_at_even.reduce_min() == min_at_even);

  // --- lvalue mask stored by reference ---
  ekat::Mask<N> ref_mask (true);
  p2 = p1;
  auto ref_w = where(ref_mask, p2);
  ref_w = T(99);
  for (int i = 0; i < N; ++i)
    REQUIRE (p2[i] == T(99));       // all slots updated
  ref_mask = ekat::Mask<N>(false);  // change original mask
  ref_w = T(0);                     // should have no effect (mask is now false)
  for (int i = 0; i < N; ++i)
    REQUIRE (p2[i] == T(99));       // unchanged

  // --- math functions (applied only where mask is true) ---
  for (int i = 0; i < N; ++i)
    p2[i] = T(i+1) * T(i+1);  // perfect squares: 1, 4, 9, ...
  auto p_even_sq = where(even, p2);

  // sqrt: masked entries should become i+1, unmasked unchanged
  PT sq_result = ekat::sqrt(p_even_sq);
  for (int i = 0; i < N; ++i) {
    if (i % 2 == 0)
      REQUIRE (std::abs(sq_result[i] - T(i+1)) < T(1e-10));
    else
      REQUIRE (sq_result[i] == p2[i]);  // unmasked: original value kept
  }

  // pow(where_expr, exp): only masked entries raised to exponent
  PT pow_result = ekat::pow(p_even_sq, T(2));
  for (int i = 0; i < N; ++i) {
    if (i % 2 == 0)
      REQUIRE (std::abs(pow_result[i] - std::pow(p2[i], T(2))) < T(1e-6));
    else
      REQUIRE (pow_result[i] == p2[i]);
  }
}

TEST_CASE("where") {
  run_tests<double,1>();
  run_tests<double,16>();
}

TEST_CASE("benchmark") {
  using namespace ekat;
  Kokkos::Timer timer;

  // Setup
  Pack<double, 8> v1, v2;
  Mask<8> staggered_mask;
  for(int i=0; i<8; ++i) staggered_mask.set(i, i % 2 == 0);

  // version A: Old Loop
  timer.reset();
  for(int n=0; n<1000000; ++n) {
    ekat_masked_loop(staggered_mask, i) {
      v1[i] = std::abs(v1[i]);
    }
  }
  double time_old = timer.seconds();

  // version B: New where-expression
  timer.reset();
  for(int n=0; n<1000000; ++n) {
    v2 = abs(where(staggered_mask, v2));
  }
  double time_new = timer.seconds();

  std::cout << "time old: " << time_old << "\n";
  std::cout << "time new: " << time_new << "\n";
}
