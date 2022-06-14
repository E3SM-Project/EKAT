#ifndef EKAT_ARCH_HPP
#define EKAT_ARCH_HPP

#include "ekat/ekat.hpp"
#include <string>

/*
 * Architecture-related calls
 */

namespace ekat {

std::string active_avx_string();

std::string ekat_config_string();

template <typename ExeSpace>
struct OnGpu { enum : bool { value = false }; };
#ifdef EKAT_ENABLE_GPU
template <> struct OnGpu<EkatGpuSpace> { enum : bool { value = true }; };
#endif

} // namespace ekat

#endif // EKAT_ARCH_HPP
