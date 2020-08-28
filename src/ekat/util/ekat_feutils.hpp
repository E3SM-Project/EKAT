#ifndef EKAT_FEUTILS_HPP
#define EKAT_FEUTILS_HPP

#include "ekat/ekat.hpp"

#include <cfenv>

#ifndef EKAT_HAVE_FEENABLEEXCEPT

namespace ekat {
inline int
fegetexcept (void)
{
  static fenv_t fenv;

  return fegetenv (&fenv) ? -1 : (fenv.__control & FE_ALL_EXCEPT);
}

inline int
feenableexcept (int excepts)
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

inline int
fedisableexcept (int excepts)
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

} // namespace ekat
#endif // EKAT_NEEDS_FEENABLEEXCEPT

#endif // EKAT_FEUTILS_HPP
