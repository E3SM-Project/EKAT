#ifdef _OPENMP
# include <omp.h>
#endif

#include <sstream>

#include "ekat/ekat_assert.hpp"
#include "ekat/util/ekat_feutils.hpp"
#include "ekat/util/ekat_arch.hpp"
#include "ekat/kokkos/ekat_kokkos_types.hpp"

/*
 * Implementations of ekat_arch.hpp functions.
 */

namespace ekat {

std::string active_avx_string () {
  std::string s;
#if defined __AVX512F__
  s += "-AVX512F";
#endif
#if defined __AVX2__
  s += "-AVX2";
#endif
#if defined __AVX__
  s += "-AVX";
#endif
  return s;
}

std::string ekat_config_string () {
  std::stringstream ss;
  ss << " ExecSpace name: " << DefaultDevice::execution_space::name() << "\n";
  ss << " ExecSpace initialized: " << (DefaultDevice::execution_space::impl_is_initialized() ? "yes" : "no") << "\n";
  ss << " active avx set: " << active_avx_string() << "\n"
     // << " packsize " << EKAT_PACK_SIZE
     << " compiler id: " <<
#if defined __INTEL_COMPILER
    "Intel\n";
#elif defined __INTEL_LLVM_COMPILER
    "IntelLLVM\n";
#elif defined __HIPCC__
    "AMD Clang\n";
#elif defined __GNUG__
    "GCC\n";
#else
    "unknown\n";
#endif
#ifdef EKAT_ENABLE_FPE
  auto mask = get_enabled_fpes();
  ss << " FPE support is enabled, current FPE mask: " << mask;
  if (mask==0) {
    ss << " (NONE)";
  } else {
    std::string delim = "";
    if (mask & FE_INVALID) {
      ss << delim << " FE_INVALID";
      delim = " |";
    }
    if (mask & FE_DIVBYZERO) {
      ss << delim << " FE_DIVBYZERO";
      delim = " |";
    }
    if (mask & FE_OVERFLOW) {
      ss << delim << " FE_OVERFLOW";
      delim = " |";
    }
  }
  ss << "\n";
#else
  ss << " FPE support is disabled\n";
#endif
  ss << " #host threads: " <<
#ifdef KOKKOS_ENABLE_OPENMP
         Kokkos::OpenMP::concurrency()
#elif defined _OPENMP
         omp_get_max_threads()
#else
         1
#endif
     << "\n";
  return ss.str();
}

} // namespace ekat
