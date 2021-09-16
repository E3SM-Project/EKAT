#include "ekat/ekat_session.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/util/ekat_arch.hpp"

#include <Kokkos_Core.hpp>

#include <vector>

namespace ekat_impl {
// Since we're initializing from inside a Fortran code and don't have access to
// char** args to pass to Kokkos::initialize, we need to do some work on our
// own. As a side benefit, we'll end up running on GPU platforms optimally
// without having to specify --kokkos-ndevices on the command line.
void initialize_kokkos () {
  // This is in fact const char*, but Kokkos::initialize requires char*.
  std::vector<char*> args;

  //   This is the only way to get the round-robin rank assignment Kokkos
  // provides, as that algorithm is hardcoded in Kokkos::initialize(int& narg,
  // char* arg[]). Once the behavior is exposed in the InitArguments version of
  // initialize, we can remove this string code.
  //   If for some reason we're running on a GPU platform, have Cuda enabled,
  // but are using a different execution space, this initialization is still
  // OK. The rank gets a GPU assigned and simply will ignore it.
#ifdef KOKKOS_ENABLE_CUDA
  int nd;
  const auto ret = cudaGetDeviceCount(&nd);
  if (ret != cudaSuccess) {
    // It isn't a big deal if we can't get the device count.
    nd = 1;
  }
  std::stringstream ss;
  ss << "--kokkos-ndevices=" << nd;
  const auto key = ss.str();
  std::vector<char> str(key.size()+1);
  std::copy(key.begin(), key.end(), str.begin());
  str.back() = 0;
  args.push_back(const_cast<char*>(str.data()));
#endif

  const char* silence = "--kokkos-disable-warnings";
  args.push_back(const_cast<char*>(silence));

  int narg = args.size();
  Kokkos::initialize(narg, args.data());
}

} // anonymous namespace

namespace ekat {

void initialize_ekat_session (bool print_config) {
  enable_default_fpes ();

  if (!Kokkos::is_initialized()) {
    if (print_config) printf("Calling initialize_kokkos\n");
    ekat_impl::initialize_kokkos();
  }

  if (print_config)
    std::cout << ekat_config_string() << "\n";
}

void initialize_ekat_session (int argc, char **argv, bool print_config) {
  enable_default_fpes ();
  Kokkos::initialize(argc, argv);
  if (print_config)
    std::cout << ekat_config_string() << "\n";
}

extern "C" {
void finalize_ekat_session () {
  Kokkos::finalize();
}
} // extern "C"

} // namespace ekat
