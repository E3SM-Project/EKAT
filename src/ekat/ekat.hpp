#ifndef EKAT_HPP
#define EKAT_HPP

#include "ekat/ekat_config.h"

#include <Kokkos_Core.hpp>

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

#ifdef EKAT_ENABLE_GPU
# if defined KOKKOS_ENABLE_CUDA
typedef Kokkos::Cuda EkatGpuSpace;
# elif defined KOKKOS_ENABLE_HIP
typedef Kokkos::Experimental::HIP EkatGpuSpace;
# elif defined KOKKOS_ENABLE_SYCL
typedef Kokkos::Experimental::SYCL EkatGpuSpace;
# else
error "EKAT does not recognize a GPU space other than Cuda, HIP and SYCL".
# endif
#endif

} // namespace ekat

#endif // EKAT_HPP
