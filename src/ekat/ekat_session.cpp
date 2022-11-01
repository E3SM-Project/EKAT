#include "ekat/ekat.hpp"
#include "ekat/ekat_session.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/util/ekat_arch.hpp"

#include <vector>
#include <cfenv>

namespace ekat_impl {

#ifdef EKAT_ENABLE_FPE_DEFAULT_MASK
static int get_default_fpes () {
  return (FE_DIVBYZERO |
          FE_INVALID   |
          FE_OVERFLOW);
  return 0;
}
#endif

// If we initialize from inside a Fortran code, we don't have access to
// char** args to pass to Kokkos::initialize. In this case we need to do
// some work on our own. As a side benefit, we'll end up running on GPU
// platforms optimally without having to specify --kokkos-num-devices on
// the command line.
// We also will use this function to generate kokkos args if the user
// specified none.
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
#ifdef EKAT_ENABLE_GPU
  int nd;
# if defined KOKKOS_ENABLE_CUDA
  const auto ret = cudaGetDeviceCount(&nd);
  const bool ok = ret == cudaSuccess;
# elif defined KOKKOS_ENABLE_HIP
  const auto ret = hipGetDeviceCount(&nd);
  const bool ok = ret == hipSuccess;
# elif defined KOKKOS_ENABLE_SYCL
  nd = 0;
  auto gpu_devs = sycl::device::get_devices(sycl::info::device_type::gpu);
  for (auto &dev : gpu_devs) {
    if (dev.get_info<sycl::info::device::partition_max_sub_devices>() > 0) {
      auto subDevs = dev.create_sub_devices<sycl::info::partition_property::partition_by_affinity_domain>(sycl::info::partition_affinity_domain::numa);
      nd += subDevs.size();
    } else {
      nd++;
    }
  }
  const bool ok = true;
# else
  error "No valid GPU space, yet EKAT_ENABLE_GPU is defined."
# endif  
  if (not ok) {
    // It isn't a big deal if we can't get the device count.
    nd = 1;
  }
  std::stringstream ss;
  ss << "--kokkos-num-devices=" << nd;
  const auto key = ss.str();
  std::vector<char> str(key.size()+1);
  std::copy(key.begin(), key.end(), str.begin());
  str.back() = 0;
  args.push_back(const_cast<char*>(str.data()));
#endif

  const char* silence = "--kokkos-disable-warnings";
  args.push_back(const_cast<char*>(silence));

  int narg = args.size();
  // Kokkos::Impl::parse_command_line_arguments in >= 3.7.00 expects argv to
  // have one more entry than narg, as documented in a comment: "Note that argv
  // has (argc + 1) arguments, the last one always being nullptr." This is safe
  // to insert for < 3.7.00, as well, so no KOKKOS_VERSION switch is needed.
  args.push_back(nullptr);
  Kokkos::initialize(narg, args.data());
}

} // anonymous namespace

namespace ekat {

void initialize_ekat_session (bool print_config) {
  std::vector<char*> args;
  initialize_ekat_session(args.size(), args.data(), print_config);
}

void initialize_ekat_session (int argc, char **argv, bool print_config) {

  if (!Kokkos::is_initialized()) {
    // If user has not specified any args containing "--kokkos",
    // set so that the code runs on GPU platforms optimally, getting the
    // round-robin rank assignment Kokkos provides, and disable
    // warnings. If the user has specified at least one arg containing
    // "--kokkos", we assume they know what they are doing.
    bool found_kokkos_args = false;
    for (int n=0; n<argc; ++n) {
      std::string arg = argv[n];
      if (arg.find("--kokkos") != std::string::npos) {
        found_kokkos_args = true;
        break;
      }
    }
    if (!found_kokkos_args) {
      if (print_config) printf("Calling initialize_kokkos\n");
      ekat_impl::initialize_kokkos();
    } else {
      Kokkos::initialize(argc, argv);
    }
  }

#ifdef EKAT_ENABLE_FPE_DEFAULT_MASK
  enable_fpes(ekat_impl::get_default_fpes());
#endif

  if (print_config) std::cout << ekat_config_string() << "\n";
}

extern "C" {
void finalize_ekat_session () {
  Kokkos::finalize();
}
} // extern "C"

} // namespace ekat
