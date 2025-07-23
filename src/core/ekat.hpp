#ifndef EKAT_HPP
#define EKAT_HPP

/*
 * This header doesn't do much as of now.
 * It just defines whether the default behavior of certain utilities
 * should ensure bfb-ness or not. E.g., it can be used to decide whether
 * reduction should guarantee reproducibility at the price of performance,
 * which can be helpful in certain unit tests.
 */

namespace ekat {

#ifdef EKAT_DEFAULT_BFB
static constexpr bool ekatBFB = true;
#else
static constexpr bool ekatBFB = false;
#endif

} // namespace ekat

#endif // EKAT_HPP
