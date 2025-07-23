#include "ekat_fpe.hpp"

#include <cfenv>

#ifndef EKAT_HAVE_FEENABLEEXCEPT

// Drop-in replacement of some useful GNU utils (needed on Apple platforms)
inline int fegetexcept (void)
{
  static fenv_t fenv;

  return fegetenv (&fenv) ? -1 : (fenv.__control & FE_ALL_EXCEPT);
}

inline int feenableexcept (int excepts)
{
  static fenv_t fenv;
  int new_excepts = excepts & FE_ALL_EXCEPT,
               old_excepts;  // previous masks

  if ( fegetenv (&fenv) ) return -1;
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // unmask
  fenv.__control &= ~new_excepts;
  fenv.__mxcsr   &= ~(new_excepts << 7);

  return ( fesetenv (&fenv) ? -1 : old_excepts );
}

inline int fedisableexcept (int excepts)
{
  static fenv_t fenv;
  int new_excepts = excepts & FE_ALL_EXCEPT,
               old_excepts;  // all previous masks

  if ( fegetenv (&fenv) ) return -1;
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // mask
  fenv.__control |= new_excepts;
  fenv.__mxcsr   |= new_excepts << 7;

  return ( fesetenv (&fenv) ? -1 : old_excepts );
}

#endif // EKAT_NEEDS_FEENABLEEXCEPT

namespace ekat {

void enable_fpes (const int mask) {
  // Make sure we don't throw because one of those exceptions
  // was already set, due to previous calculations
  feclearexcept(mask);

  // Set the new mask
  feenableexcept(mask);
}

void disable_fpes (const int mask) {
  feclearexcept(mask);
  fedisableexcept(mask);
}

int get_enabled_fpes () {
  return fegetexcept();
}

void disable_all_fpes () {
  disable_fpes(FE_ALL_EXCEPT);
}

std::string fpe_config_string () {
  auto mask = get_enabled_fpes();
  std::string cfg;
  if (mask==0) {
    cfg += "NONE";
  } else {
    int found = 0;
    std::string delim = "";
    if (mask & FE_INVALID) {
      cfg += delim + "FE_INVALID";
      delim = " |";
      found |= FE_INVALID;
    }
    if (mask & FE_DIVBYZERO) {
      cfg += delim + " FE_DIVBYZERO";
      delim = " |";
      found |= FE_DIVBYZERO;
    }
    if (mask & FE_OVERFLOW) {
      cfg += delim + " FE_OVERFLOW";
      delim = " |";
      found |= FE_OVERFLOW;
    }
    if (mask & FE_UNDERFLOW) {
      cfg += delim + " FE_UNDERFLOW";
      delim = " |";
      found |= FE_UNDERFLOW;
    }
    if (mask & FE_INEXACT) {
      cfg += delim + " FE_INEXACT";
      delim = " |";
      found |= FE_INEXACT;
    }

    // In case there's a flag we did not recognize, let the user know
    if (mask != found) {
      cfg += delim + " OTHER";
    }
  }
  cfg += "\n";

  return cfg;
}

} // namespace error
