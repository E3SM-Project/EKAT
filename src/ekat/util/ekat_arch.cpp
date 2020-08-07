#ifdef _OPENMP
# include <omp.h>
#endif

#include <sstream>

#include "ekat/util/ekat_arch.hpp"
#include "ekat/ekat_types.hpp"

/*
 * Implementations of ekat_arch.hpp functions.
 */

namespace ekat {
namespace util {

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

std::string config_string () {
  std::stringstream ss;
  ss << "ExecSpace name: " << DefaultDevice::execution_space::name() << "\n";
  ss << "ExecSpace initialized: " << (DefaultDevice::execution_space::impl_is_initialized() ? "yes" : "no") << "\n";
  ss << "sizeof(Real) " << sizeof(Real)
     << " avx " << active_avx_string()
     // << " packsize " << EKAT_PACK_SIZE
     << " compiler " <<
#if defined __INTEL_COMPILER
    "Intel"
#elif defined __GNUG__
    "GCC"
#else
    "unknown"
#endif
     << " FPE " <<
#ifdef EKAT_FPE
    "on"
#else
    "off"
#endif
     << " #threads " <<
#ifdef KOKKOS_ENABLE_OPENMP
         Kokkos::OpenMP::concurrency()
#elif defined _OPENMP
         omp_get_max_threads()
#else
         1
#endif
    ;
  return ss.str();
}

} // namespace util
} // namespace ekat
