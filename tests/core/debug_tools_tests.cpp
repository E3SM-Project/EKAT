#include <catch2/catch.hpp>

#include "ekat_assert.hpp"
#include "ekat_string_utils.hpp"
#include "ekat_fpe.hpp"

#include <csignal>
#include <unistd.h>
#include <csetjmp>
#include <iostream>

// To manually access some FPE stuff
#include <cfenv>

namespace {

#ifndef NDEBUG
static volatile std::sig_atomic_t gSignalStatus = 0;

// Force the compiler to completely separate these operations
__attribute__((noinline)) double do_div(double a, double b) { return a / b; }
__attribute__((noinline)) double do_sqrt(double a) { return std::sqrt(a); }
__attribute__((noinline)) double do_exp(double a) { return std::exp(a); }

// 1. CHANGER THE SIGNAL HANDLER TO THROW AN EXCEPTION
void signal_handler (int /* signum */) {
  gSignalStatus = 1;
  throw std::runtime_error("FPE Intercepted");
}

int has_fe_divbyzero (const int mask) {
  return (mask & FE_DIVBYZERO ? 1 : 0);
}

int has_fe_overflow (const int mask) {
  return (mask & FE_OVERFLOW ? 1 : 0);
}

int has_fe_invalid (const int mask) {
  return (mask & FE_INVALID ? 1 : 0);
}

int run_fpe_tests () {
  const int mask = ekat::get_enabled_fpes();

  std::cout << " tests mask: " << mask << "\n";

  std::fenv_t fenv;
  feholdexcept(&fenv);
  fesetenv(&fenv);

  std::cout << "   has FE_DIVBYZERO: " << has_fe_divbyzero(mask) << "\n";
  std::cout << "   has FE_INVALID:   " << has_fe_invalid(mask)   << "\n";
  std::cout << "   has FE_OVERFLOW:  " << has_fe_overflow(mask)  << "\n";

  volatile double one = 1.0;
  volatile double zero = 0.0;
  volatile double inf, nan, ovfl;
  int ntests = 0;

  // --- Test 1/0 ---
  try {
    inf = do_div(one,zero);
    inf *= 1.0;
    REQUIRE(!has_fe_divbyzero(mask));
  } catch (const std::runtime_error&) {
    // If it caught an exception, it must have been enabled.
    REQUIRE(has_fe_divbyzero(mask));
    REQUIRE(gSignalStatus == 1);
    printf ("  - 1/0 threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
    std::feclearexcept(FE_ALL_EXCEPT);
  }

  // --- Test 0/0 ---
  try {
    nan = do_div(zero,zero);
    REQUIRE(!has_fe_invalid(mask));
  } catch (const std::runtime_error&) {
    REQUIRE(has_fe_invalid(mask));
    REQUIRE(gSignalStatus == 1);
    printf ("  - 0/0 threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
    std::feclearexcept(FE_ALL_EXCEPT);
  }

  // --- Test invalid arg ---
  try {
    nan = do_sqrt(-one);
    nan *= 1.0;
    REQUIRE(!has_fe_invalid(mask));
  } catch (const std::runtime_error&) {
    REQUIRE(has_fe_invalid(mask));
    REQUIRE(gSignalStatus == 1);
    printf ("  - Invalid arg threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
    std::feclearexcept(FE_ALL_EXCEPT);
  }

  // --- Test overflow ---
  try {
    ovfl = do_exp(710*one);
    ovfl = exp(710.0);
    ovfl *= 1.0;
    REQUIRE(!has_fe_overflow(mask));
  } catch (const std::runtime_error&) {
    REQUIRE(has_fe_overflow(mask));
    REQUIRE(gSignalStatus == 1);
    printf ("  - Overflow threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
    std::feclearexcept(FE_ALL_EXCEPT);
  }

  std::cout << inf << std::endl;
  std::cout << nan << std::endl;
  std::cout << ovfl << std::endl;

  return ntests;
}

TEST_CASE ("fpes","") {
  using namespace ekat;

  struct sigaction sa;
  sa.sa_handler = signal_handler;
  // Note: We leave SA_NODEFER so that consecutive traps can fire
  // without the signal block masking subsequent tests.
  sa.sa_flags = SA_NODEFER;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGFPE, &sa, NULL);

  SECTION ("default-fpes") {
    printf ("*) testing default fpes...\n");
    feclearexcept(FE_ALL_EXCEPT);
    const int mask = ekat::get_enabled_fpes();
    int num_expected_fpes = has_fe_divbyzero(mask) +
                            has_fe_invalid(mask)*2 +
                            has_fe_overflow(mask);
    int num_actual_fpes = run_fpe_tests();
    REQUIRE (num_actual_fpes == num_expected_fpes);

    // Disable fpes before completing the section, since Section
    // destructor computes some states, including
    //   getElapsedMicroseconds()/1000000.0
    // which can cause the throw of FE_INEXACT
    disable_all_fpes();
  }

  SECTION ("user-enabled-fpes") {
    printf ("*) testing user-enabled fpes...\n");
    feclearexcept(FE_ALL_EXCEPT);
    disable_all_fpes();
    enable_fpes(FE_DIVBYZERO);
    const int mask = ekat::get_enabled_fpes();
    int num_expected_fpes = has_fe_divbyzero(mask) +
                            has_fe_invalid(mask)*2 +
                            has_fe_overflow(mask);
    int num_actual_fpes = run_fpe_tests();
    REQUIRE (num_actual_fpes == num_expected_fpes);

    // Disable fpes before completing the section, since Section
    // destructor computes some states, including
    //   getElapsedMicroseconds()/1000000.0
    // which can cause the throw of FE_INEXACT
    disable_all_fpes();
  }

  SECTION ("user-disabled-fpes") {
    printf ("*) testing user-disabled fpes...\n");
    feclearexcept(FE_ALL_EXCEPT);
    enable_fpes(FE_ALL_EXCEPT);
    disable_fpes(FE_DIVBYZERO);
    const int mask = ekat::get_enabled_fpes();
    int num_expected_fpes = has_fe_divbyzero(mask) +
                            has_fe_invalid(mask)*2 +
                            has_fe_overflow(mask);
    int num_actual_fpes = run_fpe_tests();
    REQUIRE (num_actual_fpes == num_expected_fpes);

    // Disable fpes before completing the section, since Section
    // destructor computes some states, including
    //   getElapsedMicroseconds()/1000000.0
    // which can cause the throw of FE_INEXACT
    disable_all_fpes();
  }
}
#endif

class MyException : public std::exception {
public:
  MyException(const std::string& s) : msg(s) {}
  const char* what() const noexcept override { return msg.c_str(); }

  std::string msg;
};

TEST_CASE ("assert-macros") {
  printf ("*) testing assert macros...\n");
  auto test_req_msg = [](const bool test, const std::string& msg) {
    EKAT_REQUIRE_MSG(test,msg);
  };
  auto test_err_msg = [](const std::string& msg) {
    EKAT_ERROR_MSG(msg);
  };
  auto test_with_etype = [](const bool test, const std::string& msg) {
    EKAT_REQUIRE (test,msg,MyException);
  };
  REQUIRE_THROWS (test_req_msg(1>3,"Uh? I wonder what Sharkowsky would have to say about this...\n"));

  REQUIRE_NOTHROW (test_req_msg(3>1,"Uh? I wonder what Sharkowsky would have to say about this...\n"));

  REQUIRE_THROWS (test_err_msg("Hello world!\n"));

  REQUIRE_THROWS_AS (test_with_etype(1>3,"What?"),MyException);

  // Make sure these compile
  EKAT_REQUIRE (2>0);
  EKAT_REQUIRE (2>0, "some string");
  EKAT_REQUIRE (2>0, "some string", std::logic_error);

  // Make sure the user msg does not get lost
  std::stringstream ss;
  ss << "Things went wrong...\n";
  ss << "...VERY wrong...\n";

  try {
    EKAT_ERROR_MSG(ss.str());
  } catch (std::exception& e) {
    const auto& what = e.what();
    auto lines = ekat::split(what,"\nFAILED CONDITION");
    REQUIRE (lines[0]==ss.str());
  }
}

// helper class to trigger an EKAT check during its destructor
struct UnwindTrig {
  bool should_fail;
  UnwindTrig(bool fail) : should_fail(fail) {}
  ~UnwindTrig() {
    // If should_fail is true, this will call ekat::throw_exception.
    // If this destructor is called during an unwind, throw_exception
    // should detect std::uncaught_exceptions() > 0 and log/return instead of throw.
    EKAT_REQUIRE_MSG(!should_fail, "Secondary failure in destructor");
  }
};

TEST_CASE("unwind-safety") {
  printf ("*) testing exception unwind safety...\n");

  SECTION("suppress-secondary-throw") {
    // This section verifies that if EKAT_REQUIRE fails during an unwind,
    // it doesn't throw a second exception (which would call std::terminate).

    auto trigger_unwind = []() {
      // 1. Create an object that will fail its EKAT check in its destructor
      UnwindTrig guard(true);

      // 2. Throw the "Primary" exception.
      // This starts the unwind, which calls ~UnwindTrig().
      EKAT_ERROR_MSG("Primary exception");
    };

    // If your fix works, REQUIRE_THROWS will catch the "Primary exception".
    // If it fails, the program will terminate immediately due to the secondary throw.
    REQUIRE_THROWS_WITH(trigger_unwind(), Catch::Contains("Primary exception"));

    printf("  - Successfully suppressed secondary exception during unwind.\n");
  }

  SECTION("normal-throw-after-unwind") {
    // Verify that the 'unwind_warning_issued' flag resets correctly
    // and we can still throw normally after a previous unwind event.
    try {
      UnwindTrig guard(true);
      throw std::runtime_error("First Unwind");
    } catch (...) {
      // Unwind finished
    }

    // This should still throw normally because std::uncaught_exceptions() is now 0
    bool threw = false;
    std::string my_msg = "Post-unwind failure";
    try {
      EKAT_REQUIRE_MSG(false, my_msg);
    } catch (std::runtime_error& e) {
      threw = true;
      std::string actual_err = e.what();

      // Verify the message starts correctly.
      // This ignores the volatile line number at the end of the backtrace.
      REQUIRE(ekat::starts_with(actual_err, my_msg));

      // You can also verify the failed condition is present
      REQUIRE(actual_err.find("FAILED CONDITION: 'false'") != std::string::npos);
    }
    REQUIRE (threw);
    printf("  - Correctly reset safety flag after unwind completed.\n");
  }
}

} // anonymous namespace
