include(EkatCreateUnitTest)

# Test debug tools
EkatCreateUnitTest(debug_tools debug_tools_tests.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(util_cxx util_tests.cpp
  LIBS ekat)

# Test upper_bound
EkatCreateUnitTest(upper_bound upper_bound_test.cpp
  LIBS ekat)

# Test kernel_assert
# This has to be done "manually" since if the code crashes, ctest
# will consider it a failure, regardless of whether the test property
# WILL_FAIL is set to true or not. So we build the exec manually,
# and wrap it in a bash call for the test
add_executable(kernel_assert kernel_assert.cpp)
target_link_libraries(kernel_assert ekat)

add_test(NAME kernel_assert_ut
         COMMAND sh -c "./kernel_assert | grep KERNEL_ASSERT_FAIL_TEST")
