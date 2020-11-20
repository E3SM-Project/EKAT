#ifndef EKAT_HPP
#define EKAT_HPP

#include "ekat/ekat_config.h"

/*
 * This header doesn't do much as of now. It includes ekat_config.h,
 * and declares an alias for int.
 */

namespace ekat {

using Int = int;

#ifdef EKAT_DEFAULT_BFB
static constexpr bool ekatBFB = true;
#else
static constexpr bool ekatBFB = false;
#endif

} // namespace ekat

#endif // EKAT_HPP
