#ifndef EKAT_SESSION_HPP
#define EKAT_SESSION_HPP

namespace ekat {

void initialize_ekat_session(bool print_config = true);
void initialize_ekat_session(int argc, char **argv, bool print_config = true);

// A version callable from Fortran, which can help
// in case of errors to correctly shut down Kokkos
extern "C" {
void finalize_ekat_session();
} // extern "C"
} // namespace ekat

#endif // EKAT_SESSION_HPP
