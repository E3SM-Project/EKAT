#ifndef EKAT_KOKKOS_SESSION_HPP
#define EKAT_KOKKOS_SESSION_HPP

#include <string>

namespace ekat {

void initialize_kokkos_session(bool print_config = true);
void initialize_kokkos_session(int argc, char **argv, bool print_config = true);

std::string kokkos_config_string ();

void finalize_kokkos_session();

} // namespace ekat

#endif // EKAT_SESSION_HPP
