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

// This function provides a simple and correct way to initialize Kokkos from
// any context with the correct settings.
void initialize_kokkos () {
  // Count up our devices.
  // This is the only way to get the round-robin rank assignment Kokkos
  // provides, as that algorithm is hardcoded in Kokkos::initialize(int& narg,
  // char* arg[]). Once the behavior is exposed in the InitArguments version of
  // initialize, we can remove this string code.
  //   If for some reason we're running on a GPU platform, have Cuda enabled,
  // but are using a different execution space, this initialization is still
  // OK. The rank gets a GPU assigned and simply will ignore it.
  int nd = 1;
#ifdef EKAT_ENABLE_GPU
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
#endif

  auto const settings = Kokkos::InitializationSettings()
#ifdef EKAT_ENABLE_MPI
    .set_map_device_id_by("mpi_rank")
#endif
    .set_num_devices(nd)
    .set_disable_warnings(true);
  Kokkos::initialize(settings);
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
