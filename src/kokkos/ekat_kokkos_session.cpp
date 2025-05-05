#include <Kokkos_Core.hpp>

#include <cstdio>

namespace ekat {

namespace ekat_impl {
// This function provides a simple and correct way to initialize Kokkos from
// any context with the correct settings.
void initialize_kokkos () {
  auto const settings = Kokkos::InitializationSettings()
    .set_map_device_id_by("mpi_rank") // If building without MPI, this is still fine
    .set_disable_warnings(true);
  Kokkos::initialize(settings);
}
} // namespace ekat_impl

void initialize_kokkos_session (bool print_config) {
  initialize_kokkos_session(0,nullptr, print_config);
}

void initialize_kokkos_session (int argc, char **argv, bool print_config) {

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
    if (print_config) printf("Calling initialize_kokkos\n");

    if (!found_kokkos_args) {
      ekat_impl::initialize_kokkos();
    } else {
      Kokkos::initialize(argc, argv);
    }
  }

  if (print_config) {
    printf(" Default Execution Space name: %s\n", DefaultDevice::execution_space::name());
    printf(" Default Execution Space initialized: %s\n", DefaultDevice::execution_space::impl_is_initialized() ? "yes" : "no");

#ifdef KOKKOS_ENABLE_OPENMP
    int num_host_threads = Kokkos::OpenMP().concurrency();
#elif defined _OPENMP
    int num_host_threads = omp_get_max_threads();
#else
    int num_host_threads = 1;
#endif
    printf(" #host threads: %d\n", num_host_threads);
  }
}

void finalize_ekat_session () {
  if (Kokkos::is_initialized()) {
    Kokkos::finalize();
  }
}

} // namespace ekat
