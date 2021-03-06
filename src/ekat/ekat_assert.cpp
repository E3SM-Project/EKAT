#include <iostream>

#include <mpi.h>

#include "ekat_assert.hpp"
#include "ekat_session.hpp"
#include "util/ekat_feutils.hpp"

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
#ifdef EKAT_FPE
  return (FE_DIVBYZERO |
          FE_INVALID   |
          FE_OVERFLOW);
#else
  return 0;
#endif
}

void enable_fpes (const int mask) {
  // Make sure we don't throw because one of those exceptions
  // was already set, due to previous calculations
  feclearexcept(mask);

  feenableexcept(mask);
}

void disable_fpes (const int mask) {
  fedisableexcept(mask);
}

int get_enabled_fpes () {
  return fegetexcept();
}

void disable_all_fpes () {
  disable_fpes(FE_ALL_EXCEPT);
}

} // namespace error
