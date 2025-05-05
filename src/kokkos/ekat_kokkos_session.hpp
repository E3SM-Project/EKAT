#ifndef EKAT_KOKKOS_SESSION_HPP
#define EKAT_KOKKOS_SESSION_HPP

namespace ekat {

void initialize_kokkos(bool print_config = true);
void initialize_kokkos(int argc, char **argv, bool print_config = true);

void finalize_kokkos();

} // namespace ekat

#endif // EKAT_SESSION_HPP
