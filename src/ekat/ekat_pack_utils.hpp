#ifndef EKAT_PACK_UTILS_HPP
#define EKAT_PACK_UTILS_HPP

#include "ekat/ekat_pack.hpp"
#include "ekat/ekat_assert.hpp"

#include "Kokkos_Pair.hpp"

namespace ekat {

/*
 * A small struct to help with pack indexing arithmetics
 *
 * The struct offers a bunch of static methods, that can be
 * used to go from unpacked indexing to packed indexing.
 * For instance, we can check which pack and/or which entry
 * within a pack does an array index i map to, or viceversa.
 *
 * Note: all methods are marker constexpr, so they can be
 *       used inside template/constexpr constructs
 * Note: NO CHECKS are performed on the inputs to the functions.
 */

template<int PackSize>
struct PackInfo {
  // Expose the pack size
  enum : int { N = PackSize };

  static_assert(N>0, "Error! Pack size must be positive.\n");
  // This seems funky, but write down a pow of 2 and a non-pow of 2 in binary (both positive),
  // and you'll see why it works
  static_assert ((N & (N-1))==0, "Error! We only support packs with length = 2^n, n>=0.\n");

  // Number of packs needed for a given array length
  KOKKOS_INLINE_FUNCTION
  constexpr static int num_packs (const int array_length) {
    return (array_length + N - 1) / N;
  }

  // Whether padding is needed in order to pack an array
  KOKKOS_INLINE_FUNCTION
  constexpr static bool has_padding (const int array_length) {
    return array_length % N != 0;
  }

  // The amount of padding added to the last pack
  KOKKOS_INLINE_FUNCTION
  constexpr static int padding (const int array_length) {
    return num_packs(array_length)*N - array_length;
  }

  // The end of the last pack
  KOKKOS_INLINE_FUNCTION
  constexpr static int last_vec_end (const int array_length) {
    return N-padding(array_length);
  }

  // The end of a given pack. It is N for all packs, except possibly
  // for the last (if padding>0)
  KOKKOS_INLINE_FUNCTION
  constexpr static int vec_end (const int array_length, const int pack_idx) {
    return (pack_idx+1)<num_packs(array_length) ? N : last_vec_end(array_length);
  }

  // Idx of the last pack
  KOKKOS_INLINE_FUNCTION
  constexpr static int last_pack_idx (const int array_length) {
    return num_packs(array_length)-1;
  }

  // The pack index corresponding to a given array entry
  KOKKOS_INLINE_FUNCTION
  constexpr static int pack_idx (const int idx) {
    return idx / N;
  }

  // The index within a pack corresponding to a given array entry
  KOKKOS_INLINE_FUNCTION
  constexpr static int vec_idx (const int idx) {
    return idx % N;
  }

  // A combo version of the previous two
  KOKKOS_INLINE_FUNCTION
  constexpr static Kokkos::pair<int,int> pack_vec_idx (const int idx) {
    return Kokkos::make_pair(idx / N, idx % N);
  }

  // Get the array index given pack and vec idx
  KOKKOS_INLINE_FUNCTION
  constexpr static int array_idx (const int pack_idx, const int vec_idx) {
    return pack_idx*N + vec_idx;
  }

  // Like the above one, but accepting a pair-like struct
  // NOTE: when called on device, PairType *must* be a Kokkos::pair.
  //       However, if used on host, std::pair will work too
  //       Also, the pair value cannot be a template parameter,
  //       due to limitations on non-type template parameters.
  template<typename PairType>
  KOKKOS_INLINE_FUNCTION
  constexpr static int array_idx (const PairType& pack_vec_idx) {
    return pack_vec_idx.first*N + pack_vec_idx.second;
  }
};

} // namespace ekat

#endif // EKAT_PACK_UTILS_HPP
