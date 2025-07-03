#ifndef EKAT_KOKKOS_ASSERT_HPP
#define EKAT_KOKKOS_ASSERT_HPP

// SYCL cannot printf like the other backends quite yet
#ifdef __SYCL_DEVICE_ONLY__
#define IMPL_KERNEL_THROW(condition, msg)				\
  do {									\
    if ( ! (condition) ) {						\
      const __attribute__((opencl_constant)) char format[] = "KERNEL CHECK FAILED:\n   %s %s\n"; \
      sycl::ext::oneapi::experimental::printf(format,#condition,msg);	\
      Kokkos::abort("");						\
    }									\
  } while (0)
#else
#define IMPL_KERNEL_THROW(condition, msg)				\
  do {									\
    if ( ! (condition) ) {						\
      printf("KERNEL CHECK FAILED:\n   %s\n   %s\n",#condition,msg);	\
      Kokkos::abort("");						\
    }									\
  } while (0)
#endif

#define EKAT_KERNEL_REQUIRE(condition)          IMPL_KERNEL_THROW(condition, "")
#define EKAT_KERNEL_REQUIRE_MSG(condition, msg) IMPL_KERNEL_THROW(condition, msg)

#ifndef NDEBUG
#define EKAT_KERNEL_ASSERT_MSG(condition, msg)  EKAT_KERNEL_REQUIRE_MSG(condition, msg)
#else
#define EKAT_KERNEL_ASSERT_MSG(condition, msg) ((void) (0))
#endif

#define EKAT_KERNEL_ASSERT(condition)           EKAT_KERNEL_ASSERT_MSG(condition, "")
#define EKAT_KERNEL_ERROR_MSG(msg)              EKAT_KERNEL_REQUIRE_MSG(false, msg)

#endif // EKAT_KOKKOS_ASSERT_HPP
