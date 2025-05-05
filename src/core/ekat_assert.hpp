#ifndef EKAT_ASSERT_HPP
#define EKAT_ASSERT_HPP

#include <sstream>
#include <iostream>
#include <stdexcept>  // For std::runtime_error
#include <type_traits>

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

namespace ekat {
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

} // namespace ekat

// Internal do not call directly.
#define IMPL_THROW(condition, msg, exception_type)  \
  do {                                                    \
    if ( ! (condition) ) {                                \
      std::stringstream _ss_;                             \
      _ss_ << "\n FAIL:\n" << #condition  << "\n";        \
      _ss_ << EKAT_BACKTRACE;                             \
      _ss_ << "\n" << msg;                                \
      ekat::throw_exception<exception_type>(_ss_.str());  \
    }                                                     \
  } while(0)

// Define the EKAT_REQUIRE macros for different argument counts
#define EKAT_REQUIRE_3(cond, msg, etype) IMPL_THROW(cond, msg, etype)
#define EKAT_REQUIRE_2(cond, msg)        IMPL_THROW(cond, msg, std::runtime_error)
#define EKAT_REQUIRE_1(cond)             IMPL_THROW(cond, "" , std::runtime_error)

#define EKAT_GET_REQUIRE_MACRO(_1, _2, _3, NAME, ...) NAME
#define EKAT_REQUIRE(...) EKAT_GET_REQUIRE_MACRO(__VA_ARGS__, EKAT_REQUIRE_3, EKAT_REQUIRE_2, EKAT_REQUIRE_1)(__VA_ARGS__)

#define EKAT_REQUIRE_MSG(condition,msg)  EKAT_REQUIRE(condition,msg)
#define EKAT_ERROR_MSG(msg)              EKAT_REQUIRE_MSG(false, msg)

// Unlike REQUIRE, the ASSERT macros are only doing something in DEUBG builds
#ifndef NDEBUG
#define EKAT_ASSERT_MSG(condition, msg)  EKAT_REQUIRE(condition, msg, std::runtime_error)
#else
#define EKAT_ASSERT_MSG(condition, msg)  ((void) (0))
#endif

#define EKAT_ASSERT(condition)           EKAT_ASSERT_MSG(condition, "")

#endif // EKAT_ASSERT_HPP
