#ifndef EKAT_FPE_HPP
#define EKAT_FPE_HPP

#include <string>

namespace ekat {

void enable_fpes (const int mask);
void disable_fpes (const int mask);

int get_enabled_fpes ();
void disable_all_fpes ();

std::string fpe_config_string ();

} // namespace ekat

#endif // EKAT_FPE_HPP
