#ifndef EKAT_TEST_UTILS_HPP
#define EKAT_TEST_UTILS_HPP

#include "ekat/ekat_pack.hpp"

#include <map>
#include <list>

namespace ekat {

struct TestSession {
  static TestSession& get () {
    static TestSession s;
    return s;
  }

  void parse_test_args (const std::vector<char*>& args);

  // These three store test args, but in different ways
  //  - flags contains args of the form "-s" or "--p" (not followed by another string) as a map string->bool
  //  - params contains args of the form "-p value"/"--param value"
  //  - vec_params contains args of the form "-p val1 val2 val3"/"--param val1 val2 val3"
  // Notice that, as such, params entries will ALWAYS be in vec_params as well (with legnth 1)
  std::map<std::string,bool>                      flags;
  std::map<std::string,std::string>               params;
  std::map<std::string,std::vector<std::string>>  vec_params;
private:
  TestSession() = default;
};


template <typename RealType, typename rngAlg, typename PDF>
void genRandArray(RealType *const x, int length, rngAlg &engine, PDF &&pdf) {
  for (int i = 0; i < length; ++i) {
    x[i] = pdf(engine);
  }
}

template <typename rngAlg, typename PDF, typename ScalarType, int N>
void genRandArray(Pack<ScalarType,N> *const x, int length, rngAlg &engine, PDF &&pdf) {
  for (int i = 0; i < length; ++i) {
    for (int j = 0; j < N; ++j) {
      x[i][j] = pdf(engine);
    }
  }
}

template <typename ViewType, typename rngAlg, typename PDF>
typename std::enable_if<Kokkos::is_view<ViewType>::value, void>::type
genRandArray(ViewType view, rngAlg &engine, PDF &&pdf) {
  typename ViewType::HostMirror mirror = Kokkos::create_mirror_view(view);
  genRandArray(mirror.data(), view.size(), engine, pdf);
  Kokkos::deep_copy(view, mirror);
}

// Do an == check between a scalar result and a packed-C++ result.
// Expect BFB except when C++ pksize > 1 and fp model is not strict
template <bool StrictFP, int Packsize, typename Scalar>
void catch2_req_pk_sensitive(const Scalar lhs, const Scalar rhs)
{
  if (StrictFP) {
    REQUIRE(lhs == rhs);
  } else {
    if (Packsize > 1) {
      REQUIRE(lhs == Approx(rhs));
    } else {
      REQUIRE(lhs == rhs);
    }
  }
}

// Check whether a string (from argv) matches a predefined option name,
// either in its short or long form. It is meant to check something like this:
//
//   bool is_num_problems = argv_matches(argv[1], "-np", "--num_problems");
//
// Additionally, we also check if the input matches the short option name with an extra '-',
// so in the above example we would also check "--np".
bool argv_matches(const std::string& str, const std::string& short_opt, const std::string& long_opt);

// This routine tries to detect the device id that the current MPI rank
// should use during a test.
// The routine relies on CTest RESOURCE_GROUPS feature. In particular,
// IF one specify resource groups properties when adding the test during
// cmake configuration, AND IF one passes a resource specification file
// to ctest, THEN ctest sets some env variables, which the test can read
// to figure out the resources its running on.
//
// For more info, see 
//    https://cmake.org/cmake/help/latest/prop_test/RESOURCE_GROUPS.html
//    https://cmake.org/cmake/help/latest/manual/ctest.1.html#ctest-resource-environment-variables
//
// Note 1: the name of resources is completely meaningless to ctest.
//         Here we use 'devices', but it's not necessary.
// Note 2: CTest uses the resources specs also to schedule tests execution:
//         knowing the amount of resource available (through the spec file),
//         as well as the resources requested by each test, it can make sure
//         all resources are used, without oversubscribing them.
// Note 3: This feature is only available since CMake 3.16. However, the cmake
//         logic is completely innocuous with older cmake versions, and since
//         older versions do not set any CTEST_XYZ env variable, this function
//         will simply return -1 also on CUDA, which means that Kokkos will
//         autonomously decide which device to use on GPU (usually, the first one).
// Note 4: it only does something on CUDA builds; with every other
//         kokkos device, it returns -1.
int get_test_device (const int mpi_rank);

} // namespace ekat

#endif // EKAT_TEST_UTILS_HPP
