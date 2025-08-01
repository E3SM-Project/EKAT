#include <catch2/catch.hpp>

#include "ekat_assert.hpp"
#include "ekat_fpe.hpp"

#include <csignal>
#include <unistd.h>
#include <csetjmp>
#include <iostream>

// To manually access some FPE stuff
#include <cfenv>

namespace {

jmp_buf JumpBuffer;

static volatile std::sig_atomic_t gSignalStatus = 0;

void signal_handler (int /* signum */) {
  gSignalStatus = 1;
  std::longjmp(JumpBuffer,gSignalStatus);
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

  // Get the current fenv.
  // Note: for some reason, each time SIGFPE is thrown or getenv
  //       is called, the fenv is reset to 0. So remember to
  //       set the fenv before each test.
  std::fenv_t fenv;
  feholdexcept(&fenv);
  fesetenv(&fenv);

  std::cout << "   has FE_DIVBYZERO: " << has_fe_divbyzero(mask) << "\n";
  std::cout << "   has FE_INVALID:   " << has_fe_invalid(mask)   << "\n";
  std::cout << "   has FE_OVERFLOW:  " << has_fe_overflow(mask)  << "\n";

  double one = 1.0;
  double zero = 0.0;
  double inf, nan, ovfl;
  int ntests = 0;

  // Run the tests.
  // Note: sometimes a FPE is not thrown when the bad number
  //       is generated, but rather the next time is used.
  //       Therefore, sometimes we multiply the result
  //       by 1.0 before testing that the FPE was thrown.

  // Test 1/0
  if (setjmp(JumpBuffer)) {
    REQUIRE (gSignalStatus==has_fe_divbyzero(mask));
    printf ("  - 1/0 threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
  } else {
    inf = one/zero;
    inf *= 1.0;
  }

  // Test 0/0
  if (setjmp(JumpBuffer)) {
    REQUIRE (gSignalStatus==1);
    printf ("  - 0/0 threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
  } else {
    nan = zero/zero;
  }

  // Test invalid arg
  if (setjmp(JumpBuffer)) {
    REQUIRE (gSignalStatus==1);
    printf ("  - Invalid arg threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
  } else {
    nan = std::sqrt(-1.0);
    nan *= 1.0;
  }

  // Test overflow
  if (setjmp(JumpBuffer)) {
    REQUIRE (gSignalStatus==1);
    printf ("  - Overflow threw.\n");
    ++ntests;
    gSignalStatus = 0;
    fesetenv(&fenv);
  } else {
    ovfl = exp(710.0);
    ovfl *= 1.0;
  }

  std::cout << inf << std::endl;
  std::cout << nan << std::endl;
  std::cout << ovfl << std::endl;

  return ntests;
}

TEST_CASE ("fpes","") {
  using namespace ekat;

  // Set a handler, which simply sets the global gSignalStatus,
  // so we can check with catch whether the FPE was raised.
  struct sigaction sa;
  sa.sa_handler = signal_handler;
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
    REQUIRE (run_fpe_tests () == num_expected_fpes);

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
    REQUIRE (run_fpe_tests () == num_expected_fpes);

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
    REQUIRE (run_fpe_tests () == num_expected_fpes);

    // Disable fpes before completing the section, since Section
    // destructor computes some states, including
    //   getElapsedMicroseconds()/1000000.0
    // which can cause the throw of FE_INEXACT
    disable_all_fpes();
  }
}

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
}

} // anonymous namespace
