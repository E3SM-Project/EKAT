#include <iostream>

#include <mpi.h>

#include "ekat_assert.hpp"
#include "ekat_session.hpp"

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

  // Check if mpi is active. If so, use MPI_Abort, otherwise, simply std::abort
  int flag;
  MPI_Initialized(&flag);
  if (flag!=0) {
    MPI_Abort(MPI_COMM_WORLD, code);
  } else {
    std::abort();
  }
}

} // namespace ekat

int get_default_fpes () {
#ifdef EKAT_ENABLE_FPE
#ifdef EKAT_ENABLE_DEFAULT_FPE
  return (FE_DIVBYZERO |
          FE_INVALID   |
          FE_OVERFLOW);
#else
  return 0;
#endif
#else
  fprintf(stderr, "ekat::error::get_default_fpes: EKAT floating point exception support is disabled!\n");
  fprintf(stderr, "ekat::error::get_default_fpes: Returning 0.\n");
  return 0;
#endif
}

void enable_fpes (const int mask) {
#ifdef EKAT_ENABLE_FPE
  // Make sure we don't throw because one of those exceptions
  // was already set, due to previous calculations
  feclearexcept(mask);

  feenableexcept(mask);
#else
  (void)mask;
  fprintf(stderr, "ekat::error::enable_fpes: EKAT floating point exception support is disabled!\n");
  fprintf(stderr, "ekat::error::enable_fpes: This call has no effect.\n");
#endif
}

void disable_fpes (const int mask) {
#ifdef EKAT_ENABLE_FPE
  fedisableexcept(mask);
#else
  (void)mask;
  fprintf(stderr, "ekat::error::disable_fpes: EKAT floating point exception support is disabled!\n");
  fprintf(stderr, "ekat::error::disable_fpes: This call has no effect.\n");
#endif
}

int get_enabled_fpes () {
#ifdef EKAT_ENABLE_FPE
  return fegetexcept();
#else
  fprintf(stderr, "ekat::error::get_enabled_fpes: EKAT floating point exception support is disabled!\n");
  fprintf(stderr, "ekat::error::get_enabled_fpes: Returning 0.\n");
  return 0;
#endif
}

void disable_all_fpes () {
#ifdef EKAT_ENABLE_FPE
  disable_fpes(FE_ALL_EXCEPT);
#else
  fprintf(stderr, "ekat::error::disable_all_fpes: EKAT floating point exception support is disabled!\n");
  fprintf(stderr, "ekat::error::disable_all_fpes: This call has no effect.\n");
#endif
}

} // namespace error
