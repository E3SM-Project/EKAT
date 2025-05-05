#ifndef EKAT_ARCH_HPP
#define EKAT_ARCH_HPP

#include <string>

// Helper functions to get a string representation of the current platform configuration

namespace ekat {

std::string active_avx_string ();
std::string compiler_id_string ();

} // namespace ekat

#endif // EKAT_ARCH_HPP
