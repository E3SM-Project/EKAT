#ifdef EKAT_HAS_KOKKOS
#include "ekat_kokkos_session.hpp"
#endif

#include "ekat_fpe.hpp"
#include "ekat_arch.hpp"

void ekat_initialize_test_session (int argc, char** argv, const bool print_config) {
#ifdef EKAT_HAS_KOKKOS
  ekat::initialize_kokkos_session (argc,argv,print_config);
#endif

#ifdef EKAT_ENABLE_FPE_DEFAULT_MASK
  int mask = FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW;
  ekat::enable_fpes(mask);
#endif

  if (print_config) {
    printf(" active avx set: %s\n", ekat::active_avx_string().c_str());
    printf(" compiler id: %s\n", ekat::compiler_id_string().c_str());
    printf(" current FPE: mask=%d, %s\n", ekat::get_enabled_fpes(), ekat::fpe_config_string().c_str());
  }
}

void ekat_finalize_test_session () {
#ifdef EKAT_HAS_KOKKOS
  ekat::finalize_kokkos_session ();
#endif
}
