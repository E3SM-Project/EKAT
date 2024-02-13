#ifndef EKAT_UPPER_BOUND_HPP
#define EKAT_UPPER_BOUND_HPP

#include "ekat/ekat.hpp"

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
template<class T>
KOKKOS_INLINE_FUNCTION
const T* upper_bound_impl(const T* first, const T* last, const T& value)
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

template<class T, int N>
KOKKOS_INLINE_FUNCTION
Pack<int,N> upper_bound_packed(const T* first, const int scalar_len, const Pack<T,N>& value)
{
  Pack<int,N> ub = scalar_len;
  Pack<int,N> lb = 0;
  Pack<int,N> mid;

  Mask<N> gt;
  auto check_mid = [&]() {
    vector_simd
    for (int i=0; i<N; ++i) {
      gt[i] = *(first+mid[i])>=value[i];
    }
  };
  mid = (ub + lb) / 2;
  int count = 0;
  while ( ((mid!=ub).any() and count<(scalar_len/2 + 1) ) {
    check_mid();
    mid.store(gt,ub,lb);
    ++count;
  }
  EKAT_KERNEL_REQUIRE_MSG ( (ub==lb).all(),
    "Error! Failed to find upper bound. Perhaps input data is not sorted?");
  return mid;
}

#ifdef EKAT_ENABLE_GPU
template<class T>
KOKKOS_FORCEINLINE_FUNCTION
const T* upper_bound(const T* first, const T* last, const T& value)
{
  return upper_bound_impl(first, last, value);
}
#else
using std::upper_bound;
#endif

} // namespace ekat

#endif // EKAT_UPPER_BOUND_HPP
