#ifndef EKAT_ASSERT_HPP
#define EKAT_ASSERT_HPP

#include <sstream>
#include <iostream>
#include <exception>
#include <assert.h>
#include <stdexcept>  // For std::runtime_error
#include <type_traits>

#include "ekat/ekat_config.h"  // for EKAT_CONSTEXPR_ASSERT and EKAT_ENABLE_FPE

/*
 * Asserts and error checking macros/functions.
 *
 * ekat_k* are for error checking within kokkos kernels.
 *
 * Any check with "assert" in the name is disabled for release builds
 *
 * For _msg checks, the msg argument can contain '<<' if not a kernel check.
 */

#ifdef EKAT_HAS_STACKTRACE
#include <boost/stacktrace.hpp>
#define EKAT_BACKTRACE boost::stacktrace::stacktrace()
#else
#define EKAT_BACKTRACE __FILE__ << ":" << __LINE__
#endif

// To be used in the following macro. We need template so that "if constepxr" can
// effectively PREVENT the compilation of the wrong branch. Without template indirection,
// the compiler will attempt to instantiate BOTH branches
template<typename exception_type>
void throw_exception(const std::string& msg)
{
  if constexpr (std::is_constructible<exception_type, const std::string&>::value) {
    throw exception_type(msg);
  } else if constexpr (std::is_default_constructible<exception_type>::value) {
    std::cerr << msg;
    throw exception_type();
  } else {
    std::cerr << msg << "\n";
    std::cerr << "Cannot create exception of type\n";
    std::cerr << "       " << typeid(exception_type).name() << "\n";
    std::cerr << "The program will terminate\n";
  }
}

// Internal do not call directly.
// NOTE: the ... at the end is to allow using EKAT_REQUIRE with
//       a variadic number of args, adding placeholders at the end
//       to ensure that the call to IMPL_THROW matches the signature
#define IMPL_THROW(condition, msg, exception_type)  \
  do {                                                  \
    if ( ! (condition) ) {                              \
      std::stringstream _ss_;                           \
      _ss_ << "\n FAIL:\n" << #condition  << "\n";      \
      _ss_ << EKAT_BACKTRACE;                           \
      _ss_ << "\n" << msg;                              \
      throw_exception<exception_type>(_ss_.str());      \
    }                                                   \
  } while(0)

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

#ifndef NDEBUG
#define EKAT_ASSERT_MSG(condition, msg)             IMPL_THROW(condition, msg, std::runtime_error)
#define EKAT_KERNEL_ASSERT_MSG(condition, msg)      IMPL_KERNEL_THROW(condition, msg)
#else
#define EKAT_ASSERT_MSG(condition, msg)  ((void) (0))
#define EKAT_KERNEL_ASSERT_MSG(condition, msg) ((void) (0))
#endif

#define EKAT_ASSERT(condition)          EKAT_ASSERT_MSG(condition, "")
#define EKAT_KERNEL_ASSERT(condition)   EKAT_KERNEL_ASSERT_MSG(condition, "")

#define EKAT_COUNT_ARGS_IMPL(_1, _2, _3, N, ...) N
#define EKAT_COUNT_ARGS(...)  EKAT_COUNT_ARGS_IMPL(__VA_ARGS__, 3, 2, 1, 0)


// Define the EKAT_REQUIRE macros for different argument counts
#define EKAT_REQUIRE_3(cond, msg, etype) IMPL_THROW(cond, msg, etype)
#define EKAT_REQUIRE_2(cond, msg)        IMPL_THROW(cond, msg, std::runtime_error)
#define EKAT_REQUIRE_1(cond)             IMPL_THROW(cond, "" , std::runtime_error)

#define EKAT_GET_REQUIRE_MACRO(_1, _2, _3, NAME, ...) NAME
#define EKAT_REQUIRE(...) EKAT_GET_REQUIRE_MACRO(__VA_ARGS__, EKAT_REQUIRE_3, EKAT_REQUIRE_2, EKAT_REQUIRE_1)(__VA_ARGS__)

#define EKAT_REQUIRE_MSG(condition,msg) \
  EKAT_REQUIRE (condition,msg)

#define EKAT_KERNEL_REQUIRE(condition)                IMPL_KERNEL_THROW(condition, "")
#define EKAT_KERNEL_REQUIRE_MSG(condition, msg)       IMPL_KERNEL_THROW(condition, msg)

#define EKAT_ERROR_MSG(msg)                           EKAT_REQUIRE_MSG(false, msg)
#define EKAT_KERNEL_ERROR_MSG(msg)                    EKAT_KERNEL_REQUIRE_MSG(false, msg)

// Macros to do asserts inside constexpr functions
// This is not necessary with C++14, where constexpr functions are "regular"
// functions (they just can be evaluated at compile time). But in C++11 you can only have
// 'return blah;' in a constexpr function, and user-defined constexpr constructors must
// have an empty body (only initializer list syntax allowed).
// NOTE: doing `return assert(check), blah;` seems the right solution, but it seems
//       older versions of gcc may not like this option even if check evaluates to true.
// As soon as we can use C++14, this macro can be removed, and you can simply use
// `assert(check);` in the body of the constexpr function.
#if defined(NDEBUG) || !defined(EKAT_CONSTEXPR_ASSERT)
#define CONSTEXPR_ASSERT(CHECK) void(CHECK)
#else
namespace impl {
struct assert_failure {
  explicit assert_failure(const char *sz)
  {
    std::fprintf(stderr, "Assertion failure: %s\n", sz);
  }
};
}
#define CONSTEXPR_ASSERT(CHECK) \
    ( (CHECK) || (throw impl::assert_failure(#CHECK), false) )
#endif

namespace ekat {
namespace error {

void runtime_check(bool cond, const std::string& message, int code = -1);
void runtime_abort(const std::string& message, int code = -1);

} // namespace error

/*
 * Routines to activate/deactivate floating point exceptions.
 * These routines are only meaningful if EKAT_ENABLE_FPE is defined.
 * The last two functions activate/deactivate a predefined set
 * of FPEs: FE_DIVBYZERO, FE_INVALID, and FE_OVERFLOW.
 * If you need to temporarily enable/disable a specific exception,
 * you should use the first set of functions, which accept
 * a specific fpe mask.
 *
 * WARNING: be *very* careful when using these functions, as they
 *          effectively change the FPE environment for the rest
 *          of the translation unit. If you only need to turn off
 *          FPEs for a few lines (perhaps you do some dirty trick
 *          that is correct, but would throw an fpe), don't forget
 *          to re-enable them after you're done.
 */

void enable_fpes (const int mask);
void disable_fpes (const int mask);

int get_enabled_fpes ();
void disable_all_fpes ();

} // namespace ekat

#endif // EKAT_ASSERT_HPP
