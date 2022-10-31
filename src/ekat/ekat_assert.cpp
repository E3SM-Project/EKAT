#include <iostream>

#include "ekat_assert.hpp"
#include "ekat_session.hpp"

#ifdef EKAT_ENABLE_MPI
#include <mpi.h>
#endif

#ifdef EKAT_ENABLE_FPE
#include "util/ekat_feutils.hpp"
#endif

namespace ekat {
namespace error {

void runtime_check(bool cond, const std::string& message, int code) {
  if (!cond) {
    runtime_abort(message,code);
  }
}

void runtime_abort(const std::string& message, int code) {
  std::cerr << message << std::endl << "Exiting..." << std::endl;

  // Finalize ekat (e.g., finalize kokkos);
  finalize_ekat_session();

#ifdef EKAT_ENABLE_MPI
  // Check if mpi is active. If so, use MPI_Abort, otherwise, simply std::abort
  int flag;
  MPI_Initialized(&flag);
  if (flag!=0) {
    MPI_Abort(MPI_COMM_WORLD, code);
  } else {
    std::abort();
  }
#else
  std::abort();
#endif
}

} // namespace ekat

void enable_fpes (const int mask) {
#ifdef EKAT_ENABLE_FPE
  // Make sure we don't throw because one of those exceptions
  // was already set, due to previous calculations
  feclearexcept(mask);

  feenableexcept(mask);
#else
  (void)mask;
  fprintf(stderr,
      "WARNING! EKAT floating point exception support is disabled!\n"
      "         This call to ekat::enable_fpes has no effect.\n");
#endif
}

void disable_fpes (const int mask) {
#ifdef EKAT_ENABLE_FPE
  fedisableexcept(mask);
#else
  (void)mask;
  fprintf(stderr,
      "WARNING! EKAT floating point exception support is disabled!\n"
      "         This call to ekat::disable_fpes has no effect.\n");
#endif
}

int get_enabled_fpes () {
#ifdef EKAT_ENABLE_FPE
  return fegetexcept();
#else
  fprintf(stderr,
      "WARNING! EKAT floating point exception support is disabled!\n"
      "         This call to ekat::get_enabled_fpes returns 0.\n");
  return 0;
#endif
}

void disable_all_fpes () {
#ifdef EKAT_ENABLE_FPE
  disable_fpes(FE_ALL_EXCEPT);
#else
  fprintf(stderr,
      "WARNING! EKAT floating point exception support is disabled!\n"
      "         This call to ekat::disable_all_fpes has no effect.\n");
#endif
}

} // namespace error
