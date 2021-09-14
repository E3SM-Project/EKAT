#include "catch2/catch.hpp"

#include "ekat/ekat_pack.hpp"
#include "ekat/util//ekat_math_utils.hpp"
#include "ekat/kokkos/ekat_kokkos_types.hpp"
#include "ekat_test_config.h"
#include <sstream>

namespace {

double square(double x) {
  return x * x;
}

double cube(double x) {
  return x * x * x;
}

template <int PACKN>
struct TestMask {
  using Mask = ekat::Mask<PACKN>;
  using Pack = ekat::Pack<int, PACKN>;

  static int sum_true (const Mask& m) {
    int sum1 = 0, sum2 = 0;
    ekat_masked_loop(m, s) ++sum1;
    ekat_masked_loop_no_vec(m, s) ++sum2;
    REQUIRE(sum1 == sum2);
    return sum1;
  }

  static void run () {
    {
      Mask m(false);
      REQUIRE( ! m.any());
      REQUIRE(   m.none());
    }
    {
      Mask m(true);
      REQUIRE(m.any());
      REQUIRE(!m.none());
      REQUIRE( ! ( ! m).any());
      for (int i = 0; i < Mask::n; ++i) REQUIRE(m[i]);
      REQUIRE(sum_true(m) == Mask::n);
    }
    for (int i = 0; i < Mask::n; ++i) {
      Mask m(false);
      m.set(i, true);
      REQUIRE(sum_true(m) == 1);
    }
    {
      Pack a, b;
      for (int i = 0; i < Mask::n; ++i)
        a[i] = i + (i % 2);
      for (int i = 0; i < Mask::n; ++i)
        b[i] = i;
      const auto m1 = a > b;
      ekat_masked_loop_no_vec(m1, s) REQUIRE(m1[s] == (s % 2 == 1));
      const auto m2 = b < a;
      ekat_masked_loop_no_vec(m2, s) REQUIRE(m2[s] == (s % 2 == 1));
      REQUIRE(sum_true(m1 && m2) == Mask::n / 2);
      REQUIRE(sum_true(m1 && true) == Mask::n / 2);
      REQUIRE(sum_true(m1 && false) == 0);
      REQUIRE(sum_true(m1 || true) == Mask::n);
      REQUIRE(sum_true(m1 || false) == Mask::n / 2);
      REQUIRE(sum_true(m1 || m2) == Mask::n / 2);
      REQUIRE(sum_true(m1 && ! m2) == 0);
      REQUIRE(sum_true(m1 || ! m2) == Mask::n);
    }
  }
};

TEST_CASE("Mask", "ekat::pack") {
#ifndef __INTEL_COMPILER
  TestMask<1>::run();
#endif
  TestMask<2>::run();
  TestMask<3>::run();
  TestMask<4>::run();
  TestMask<8>::run();
  TestMask<16>::run();
  TestMask<32>::run();
}

template <typename Scalar, int PACKN>
struct TestPack {
  using Mask = ekat::Mask<PACKN>;
  using Pack = ekat::Pack<Scalar, PACKN>;
  using scalar = typename Pack::scalar;

  static const double tol;

  // Use macros so that catch2 reports useful line numbers.
#define compare_packs(a, b) do {                  \
    REQUIRE(max(abs(a - b)) <= TestPack<Scalar,PACKN>::tol*max(abs(a)));  \
  } while (0)

#define compare_masks(a, b) do {                    \
    vector_novec for (int i = 0; i < Mask::n; ++i)  \
      REQUIRE(a[i] == b[i]);                        \
  } while (0)

  static void setup (Pack& a, Pack& b, scalar& c,
                     const bool limit = false, const bool pve = false) {
    using ekat::impl::min;
    vector_novec for (int i = 0; i < Pack::n; ++i) {
      const auto sign = pve ? 1 : (2*(i % 2) - 1);
      a[i] = i + 1.2;
      if (limit) a[i] = min<scalar>(scalar(3.2), a[i]);
      a[i] *= sign;
    }
    vector_novec for (int i = 0; i < Pack::n; ++i) {
      const auto sign = pve ? 1 : (2*(i % 2) - 1);
      b[i] = i - 11.4;
      if (limit) b[i] = min<scalar>(scalar(3.2), b[i]);
      if (b[i] == 0) b[i] = 2;
      b[i] *= sign;
    }
    c = scalar(limit ? 3.2 : 41.5);
  }

  static void setup_pow (Pack& a, Pack& b, scalar& c) {
    setup(a, b, c, true, true);
  }

  static void test_conversion () {
    using IntPack = ekat::Pack<ekat::Int, PACKN>;
    Pack a;
    IntPack a_int_true;
    vector_novec for (int i = 0; i < Pack::n; ++i) {
      a[i] = (i + 0.5);
      a_int_true[i] = i;
    }
    IntPack a_int(a);
    compare_packs(a_int_true, a_int);
  }

  static void test_unary_min_max () {
    Mask m(true);
    REQUIRE( ! ( ! m).any());
    Pack p;
    vector_novec for (int i = 0; i < p.n; ++i) {
      vector_novec for (int j = 0; j < p.n; ++j)
        p[j] = j;
      p[i] = -1;
      REQUIRE(min(p) == -1);
      REQUIRE(min(m, p.n, p) == -1);
      p[i] = p.n;
      REQUIRE(max(p) == p.n);
      REQUIRE(max(m, -1, p) == p.n);
    }
  }

  static void test_masked_ctor () {
    Pack p_ref;

    // Make p_ref = [1 3 1 3 ... ]
    vector_novec 
    for (int i = 0; i < PACKN; ++i) {
      p_ref[i] = 3-2*(i%2);
    }

    // Create mask and service pack
    Mask m = p_ref>2;
    Pack one = 1;
    Pack three = 3;

    // Masked ctor to create a pack with [3 1 3 1 ...]
    // We can do this with scalars or packs
    Pack p1(m,3,1);
    Pack p2(m,three,one);

    // Check
    vector_novec for (int i = 0; i < PACKN; ++i) {
      REQUIRE(p1[i] == p_ref[i]);
      REQUIRE(p2[i] == p_ref[i]);
    }

    // Masked ctor to create a pack with [3 invalid 3 invalid ...]
    // We can do this with scalars or packs
    Pack p3(m,3);
    Pack p4(m,three);

    // Check
    vector_novec for (int i = 0; i < PACKN; ++i) {
      if (i%2==0) {
        REQUIRE(p3[i] == p_ref[i]);
        REQUIRE(p4[i] == p_ref[i]);
      } else {
        REQUIRE(ekat::is_invalid(p3[i]));
        REQUIRE(ekat::is_invalid(p4[i]));
      }
    }
  }

  static void test_masked_set () {
    Pack p_ref;

    // Make p_ref = [1 3 1 3 ... ]
    vector_novec 
    for (int i = 0; i < PACKN; ++i) {
      p_ref[i] = 3-2*(i%2);
    }

    // Create mask and service pack
    Mask m = p_ref>2;
    Pack one = 1;
    Pack three = 3;

    // Masked set to create a pack with [3 1 3 1 ...]
    // We can do this in many ways

    // Construct with one value, and set the other when needed
    Pack p1 = 1;
    p1.set(m,3);
    Pack p2 = one;
    p2.set(m,three);

    // Construct default, and call set with both values
    Pack p3;
    p3.set(m,3,1);
    Pack p4;
    p4.set(m,three,one);

    // Construct directly with two values
    Pack p5(m,3,1);
    Pack p6(m,three,one);

    // Test that chaining ops to set works
    Pack p7;
    p7.set(m,three).set(!m,one);

    // Check all packs are [3 1 3 1 ...]
    vector_novec for (int i = 0; i < PACKN; ++i) {
      REQUIRE(p1[i] == p_ref[i]);
      REQUIRE(p2[i] == p_ref[i]);
      REQUIRE(p3[i] == p_ref[i]);
      REQUIRE(p4[i] == p_ref[i]);
      REQUIRE(p5[i] == p_ref[i]);
      REQUIRE(p6[i] == p_ref[i]);
      REQUIRE(p7[i] == p_ref[i]);
    }
  }

  static void test_range () {
    const auto p = ekat::range<Pack>(42);
    vector_novec for (int i = 0; i < Pack::n; ++i)
      REQUIRE(p[i] == static_cast<scalar>(42 + i));
  }

  static void test_ostream() {
    const auto p = ekat::range<Pack>(42);
    std::ostringstream ssp;
    ssp << p;
    std::ostringstream sst;
    for (int i = 0; i < Pack::n; ++i)
      sst << 42 + i << ' ';
    REQUIRE(ssp.str() == sst.str());
  }

  template<bool Serialize>
  static void test_reduce_sum () {
    scalar a = scalar(0.5);
    scalar serial_result = scalar(a);
    Pack p;
    for (int i=0; i<Pack::n; ++i) {
      p[i] = 1.0/(i+1);
      serial_result += p[i];
    }

    ekat::reduce_sum<Serialize>(p, a);

    if (Serialize) {
      REQUIRE(a == serial_result);
    } else {
      REQUIRE(std::abs(a - serial_result) <= 10*std::numeric_limits<Scalar>::epsilon());
    }
  }

#define test_pack_gen_assign_op_all(op) do {        \
    Pack a, b;                                      \
    scalar c;                                       \
    setup(a, b, c);                                 \
    const auto a0(a);                               \
    auto ac(a0);                                    \
    a op b;                                         \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      ac[i] op b[i];                                \
    compare_packs(ac, a);                           \
    a = a0;                                         \
    ac = a0;                                        \
    a op c;                                         \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      ac[i] op c;                                   \
    compare_packs(ac, a);                           \
  } while (0)

#define test_pack_gen_bin_op_all(op) do {           \
    Pack a, b, d, dc;                               \
    scalar c;                                       \
    setup(a, b, c);                                 \
    d = a op b;                                     \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      dc[i] = a[i] op b[i];                         \
    compare_packs(dc, d);                           \
    d = a op c;                                     \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      dc[i] = a[i] op c;                            \
    compare_packs(dc, d);                           \
    d = c op b;                                     \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      dc[i] = c op b[i];                            \
    compare_packs(dc, d);                           \
  } while (0)

#define test_pack_gen_bin_fn_all(op, impl, setup_fn) do { \
    Pack a, b, d, dc;                                     \
    scalar c;                                             \
    setup_fn(a, b, c);                                    \
    d = op(a, b);                                         \
    vector_novec for (int i = 0; i < Pack::n; ++i)        \
      dc[i] = impl(a[i], b[i]);                           \
    compare_packs(dc, d);                                 \
    d = op(a, c);                                         \
    vector_novec for (int i = 0; i < Pack::n; ++i)        \
      dc[i] = impl(a[i], c);                              \
    compare_packs(dc, d);                                 \
    d = op(c, b);                                         \
    vector_novec for (int i = 0; i < Pack::n; ++i)        \
      dc[i] = impl(c, b[i]);                              \
    compare_packs(dc, d);                                 \
  } while (0)

#define test_pack_gen_unary_op(op) do {             \
    Pack a, b, ac;                                  \
    scalar c;                                       \
    setup(a, b, c);                                 \
    a = op b;                                       \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      ac[i] = op b[i];                              \
    compare_packs(ac, a);                           \
  } while (0)

#define test_mask_gen_bin_op_all(op) do {           \
    Pack a, b;                                      \
    scalar c;                                       \
    Mask m, mc;                                     \
    setup(a, b, c);                                 \
    m = a op b;                                     \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      mc.set(i, a[i] op b[i]);                      \
    compare_masks(mc, m);                           \
    m = a op c;                                     \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      mc.set(i, a[i] op c);                         \
    compare_masks(mc, m);                           \
    m = c op b;                                     \
    vector_novec for (int i = 0; i < Pack::n; ++i)  \
      mc.set(i, c op b[i]);                         \
    compare_masks(mc, m);                           \
  } while (0)

  static void run () {
    test_pack_gen_assign_op_all(=);
    test_pack_gen_assign_op_all(+=);
    test_pack_gen_assign_op_all(-=);
    test_pack_gen_assign_op_all(*=);
    test_pack_gen_assign_op_all(/=);

    test_pack_gen_bin_op_all(+);
    test_pack_gen_bin_op_all(-);
    test_pack_gen_bin_op_all(*);
    test_pack_gen_bin_op_all(/);

    test_pack_gen_bin_fn_all(min, ekat::impl::min, setup);
    test_pack_gen_bin_fn_all(max, ekat::impl::max, setup);
    test_pack_gen_bin_fn_all(pow, std::pow, setup_pow);

    test_pack_gen_unary_op(-);

    test_mask_gen_bin_op_all(==);
    test_mask_gen_bin_op_all(!=);
    test_mask_gen_bin_op_all(>=);
    test_mask_gen_bin_op_all(<=);
    test_mask_gen_bin_op_all(>);
    test_mask_gen_bin_op_all(<);

    test_conversion();
    test_unary_min_max();
    test_range();
    test_ostream();
    test_masked_ctor();
    test_masked_set();

    test_reduce_sum<true>();
    test_reduce_sum<false>();
  }
};

template <typename Scalar, int PACKN>
struct TestPackMathFcn : public TestPack<Scalar,PACKN> {
  static_assert (std::is_floating_point<Scalar>::value,
                 "Error! Overload of math fcns only provided for floating point scalars.\n");
  using base = TestPack<Scalar,PACKN>;
  using Pack = typename base::Pack;
  using scalar = typename base::scalar;

#define test_pack_gen_unary_fn(op, impl) do {     \
  Pack a, b, ac;                                  \
  scalar c;                                       \
  base::setup(a, b, c, true);                     \
  a = op(abs(b));                                 \
  vector_novec for (int i = 0; i < Pack::n; ++i)  \
    ac[i] = impl(std::abs(b[i]));                 \
  compare_packs(ac, a);                           \
} while (0)

#define test_pack_gen_unary_stdfn(op)           \
  test_pack_gen_unary_fn(op, std::op)

  static void run () {
    test_pack_gen_unary_stdfn(abs);
    test_pack_gen_unary_stdfn(exp);
    test_pack_gen_unary_stdfn(expm1);
    test_pack_gen_unary_stdfn(log);
    test_pack_gen_unary_stdfn(log10);
    test_pack_gen_unary_stdfn(tgamma);
    test_pack_gen_unary_stdfn(sqrt);
    test_pack_gen_unary_stdfn(erf);

    test_pack_gen_unary_fn(square, square);
    test_pack_gen_unary_fn(cube, cube);
  }
};

template <typename Scalar, int PACKN>
const double TestPack<Scalar,PACKN>::tol =
  2*std::numeric_limits<Scalar>::epsilon();

TEST_CASE("Pack", "ekat::pack") {
  TestPack<int,EKAT_TEST_PACK_SIZE>::run();
  TestPack<long,EKAT_TEST_PACK_SIZE>::run();
  TestPack<float,EKAT_TEST_PACK_SIZE>::run();
  TestPack<double,EKAT_TEST_PACK_SIZE>::run();
  TestPackMathFcn<float,EKAT_TEST_PACK_SIZE>::run();
  TestPackMathFcn<double,EKAT_TEST_PACK_SIZE>::run();

  if (EKAT_TEST_PACK_SIZE != 1) {
#ifndef __INTEL_COMPILER
    // Intel emits "remark: simd loop has only one iteration", and
    // apparently this cannot be silenced with pragma warning, so skip
    // these in an Intel build.
    TestPack<int,1>::run();
    TestPack<long,1>::run();
    TestPack<float,1>::run();
    TestPack<double,1>::run();
    TestPackMathFcn<float,1>::run();
    TestPackMathFcn<double,1>::run();
#endif
  }
}

TEST_CASE("isnan", "ekat::pack") {
#ifdef EKAT_DOUBLE_PRECISION
  using Real = double;
#else
  using Real = float;
#endif

  using namespace ekat;
  using pt = Pack<Real, EKAT_TEST_PACK_SIZE>;
  using mt = Mask<EKAT_TEST_PACK_SIZE>;

  using pvt = typename KokkosTypes<ekat::DefaultDevice>::template view_1d<pt>;
  using mvt = typename KokkosTypes<ekat::DefaultDevice>::template view_1d<mt>;

  pvt zero("",1), nan("",1);
  mvt mzero("",1), mnan("",1);
  Kokkos::parallel_for(Kokkos::RangePolicy<>(0,1),
                       KOKKOS_LAMBDA(int) {
    zero(0) = pt(0);  // Ctor inits pack to 0
    nan(0)  = pt();   // Ctor inits pack to nan

    const pt& z = zero(0);
    const pt& n = nan(0);

    mzero(0) = isnan(z);
    mnan(0)  = isnan(n);
  });

  auto mzero_h = Kokkos::create_mirror_view(mzero);
  auto mnan_h  = Kokkos::create_mirror_view(mnan);

  Kokkos::deep_copy(mzero_h,mzero);
  Kokkos::deep_copy(mnan_h,mnan);

  const mt& mz = mzero_h(0);
  const mt& mn = mnan_h(0);
  for (int i=0; i<EKAT_TEST_PACK_SIZE; ++i) {
    REQUIRE (!mz[i]); // the view 'zero' should not contain nans
    REQUIRE (mn[i]);  // the view 'nan'  should contain nans
  }
}

} // namespace
