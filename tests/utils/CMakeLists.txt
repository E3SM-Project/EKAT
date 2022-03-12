include(EkatCreateUnitTest)

# Test debug tools
EkatCreateUnitTest(debug_tools debug_tools_tests.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(meta_utils meta_utils.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(util_cxx util_tests.cpp
  LIBS ekat)

# Test utilities (c++)
EkatCreateUnitTest(string_utils string_utils_tests.cpp
  LIBS ekat)

# Test upper_bound
EkatCreateUnitTest(upper_bound upper_bound_test.cpp
  LIBS ekat)

# Test math utils
EkatCreateUnitTest(math_util math_util_tests.cpp
  LIBS ekat)

# Test that failure are indeed spotted. Also, verify that redirection
# of input is not parsed as test filter, by fwding a file
EkatCreateUnitTest(regress_fail regress_fail.cpp
  LIBS ekat EXE_ARGS " < CTestTestfile.cmake"
  PROPERTIES WILL_FAIL true
  LABELS "MustFail"
)

# Test that Catch returns a failure if invalid flags are passed
EkatCreateUnitTestFromExec(catch_main_invalid_flags regress_fail
  EXE_ARGS " --non-existent-flag"
  PROPERTIES WILL_FAIL true
  LABELS "MustFail"
)

# Test RDC link correctness when main EXE is in F(0
add_executable(cuda_rdc cuda_rdc.F90 cuda_rdc.cpp)
add_library(cuda_kernel cuda_rdc_kernel.cpp)
target_link_libraries(cuda_kernel PUBLIC ekat)
target_link_libraries(cuda_rdc PUBLIC ekat cuda_kernel)

target_link_options(cuda_kernel PUBLIC $<DEVICE_LINK:--device-link>)
set_target_properties (cuda_kernel PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)

target_link_options(cuda_rdc PUBLIC $<DEVICE_LINK:--device-link>)
set_target_properties (cuda_rdc PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)

