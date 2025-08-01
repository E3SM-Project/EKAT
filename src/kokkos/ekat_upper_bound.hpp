#ifndef EKAT_UPPER_BOUND_HPP
#define EKAT_UPPER_BOUND_HPP

#include <Kokkos_Core.hpp>

#ifndef EKAT_ENABLE_GPU
# include <algorithm>
#endif

namespace ekat {

/*
 * The implementation below is a modified version of
 * https://en.cppreference.com/w/cpp/algorithm/upper_bound
 *
 * The content is licensed under Creative Commons
 * Attribution-Sharealike 3.0 Unported License (CC-BY-SA) and by the
 * GNU Free Documentation License (GFDL) (unversioned, with no
 * invariant sections, front-cover texts, or back-cover texts). That
 * means that you can use this site in almost any way you like,
 * including mirroring, copying, translating, etc. All we would ask is
 * to provide link back to cppreference.com so that people know where
 * to get the most up-to-date content. In addition to that, any
 * modified content should be released under an equivalent license so
 * that everyone could benefit from the modified versions.
 */

#ifdef EKAT_ENABLE_GPU
template<class T>
KOKKOS_FORCEINLINE_FUNCTION
const T* upper_bound(const T* first, const T* last, const T& value)
{
  const T* it;
  int count, step;
  count = last - first;

  while (count > 0) {
    it = first;
    step = count / 2;
    it += step;
    if (value >= *it) {
      first = ++it;
      count -= step + 1;
    }
    else {
      count = step;
    }
  }
  return first;
}
#else
using std::upper_bound;
#endif

} // namespace ekat

#endif // EKAT_UPPER_BOUND_HPP
