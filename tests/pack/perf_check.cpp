#include <catch2/catch.hpp>

#include "ekat_pack.hpp"
#include "ekat_pack_where.hpp"
#include "ekat_pack_utils.hpp"

#include <random>

TEST_CASE("perf_test") {
  using namespace ekat;
  Kokkos::Timer timer;

  std::random_device rdev;
  const int catchRngSeed = Catch::rngSeed();
  int seed = catchRngSeed==0 ? rdev()/2 : catchRngSeed;
  std::cout << "using rng seed: " << seed << " (catch seed was " << catchRngSeed << ")\n";
  std::mt19937_64 engine(seed);
  std::uniform_real_distribution<double> pdf(-1,1);

  constexpr int N = 8;
  using PackT = Pack<double,N>;
  using MaskT = Mask<N>;

  constexpr int nruns = 1000;
  constexpr int npacks = 1000;

  std::vector<PackT> pdata(npacks);
  genRandArray(pdata.data(),npacks,engine,pdf);

  std::vector<MaskT> mdata(npacks);
  for (int i=0; i<npacks; ++i) {
    // Switch to the bottom line to get ~50/50 true/false masks
    mdata[i] = MaskT(true);
    // mdata[i] = pdata[i] > 0;
  }

  // version A: masked loop
  std::vector<PackT> out1(npacks);
  timer.reset();
  for (int r=0; r<nruns; ++r) {
    for(int n=0; n<npacks; ++n) {
      PackT res;
      ekat_masked_loop(mdata[n], i) {
        res[i] = Kokkos::exp(pdata[n][i]);
      }
      out1[n] = res;
    }
  }
  double time_old = timer.seconds();

  // version B: where-expression
  std::vector<PackT> out2(npacks);
  timer.reset();
  for (int r=0; r<nruns; ++r) {
    for(int n=0; n<npacks; ++n) {
      // out2[n] = exp(pdata[n]);
      out2[n] = exp(where(mdata[n], pdata[n]));
    }
  }
  double time_new = timer.seconds();

  std::cout << "time old: " << time_old << "\n";
  std::cout << "time new: " << time_new << "\n";
}
