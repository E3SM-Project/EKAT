#include "ekat_arch.hpp"

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

std::string compiler_id_string ()
{
#if defined __INTEL_COMPILER
  return "Intel\n";
#elif defined __INTEL_LLVM_COMPILER
  return "IntelLLVM\n";
#elif defined __HIPCC__
  return "AMD Clang\n";
#elif defined __GNUG__
  return "GCC\n";
#else
  return "unknown\n";
#endif
}

} // namespace ekat
