#ifndef EKAT_KOKKOS_STR_UTILS_HPP
#define EKAT_KOKKOS_STR_UTILS_HPP

#include "ekat_assert.hpp"

#include <Kokkos_Core.hpp>

#include <cstring>

namespace ekat {

namespace impl {

#ifdef EKAT_ENABLE_GPU
// Replacements for namespace std functions that don't run on the GPU.
KOKKOS_INLINE_FUNCTION
size_t strlen(const char* str)
{
  EKAT_KERNEL_ASSERT(str != NULL);
  const char *char_ptr;
  for (char_ptr = str; ; ++char_ptr)  {
    if (*char_ptr == '\0') return char_ptr - str;
  }
}
KOKKOS_INLINE_FUNCTION
void strcpy(char* dst, const char* src)
{
  EKAT_KERNEL_ASSERT(dst != NULL && src != NULL);
  while((*dst++ = *src++));
}
KOKKOS_INLINE_FUNCTION
int strcmp(const char* first, const char* second)
{
  while(*first && (*first == *second))
  {
    first++;
    second++;
  }
  return *(const unsigned char*)first - *(const unsigned char*)second;
}
#else
using std::strlen;
using std::strcpy;
using std::strcmp;
#endif // EKAT_ENABLE_GPU

} // namespace impl

} // namespace ekat

#endif // EKAT_KOKKOS_STR_UTILS_HPP
