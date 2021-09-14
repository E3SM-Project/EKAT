#ifndef EKAT_MACROS_HPP
#define EKAT_MACROS_HPP

#if defined __INTEL_COMPILER
# define vector_simd _Pragma("omp simd")
# define vector_novec _Pragma("novector")
#elif defined(__GNUG__) && !defined(__clang__)
# define vector_simd _Pragma("omp simd")
# define vector_novec
#else
# define vector_simd
# define vector_novec
#endif

// Annotate a loop with this symbol if vector_simd should work but
// currently does not due to a compiler issue. For example,
// compilation to reduce_min seems not to work in Intel 17.
#define vector_disabled vector_novec

#endif // EKAT_MACROS_HPP
