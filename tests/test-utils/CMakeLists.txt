include(EkatCreateUnitTest)

# Test catch main options
EkatCreateUnitTest (catch_main_tests catch_main_tests.cpp
  EXE_ARGS "--args -a 1 --b 2 3 -c=4,5 --d 6,7 -K -foo --bar"
  PROPERTIES PASS_REGULAR_EXPRESSION
  ".*flags: K bar foo, params: a=1, vec_params: a=1 b=2,3 c=4,5 d=6,7")

# Test that failure are indeed spotted. Also, verify that redirection
# of input is not parsed as test filter, by fwding a file
EkatCreateUnitTest(regress_fail regress_fail.cpp
  LIBS Utils
  EXE_ARGS " < CTestTestfile.cmake"
  WILL_FAIL
  LABELS "MustFail"
)

# Test that Catch returns a failure if invalid flags are passed
EkatCreateUnitTestFromExec(catch_main_invalid_flags regress_fail
  EXE_ARGS " --non-existent-flag"
  WILL_FAIL
  LABELS "MustFail"
)

