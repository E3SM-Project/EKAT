include(EkatCreateUnitTest)

if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(pack${DP_POSTFIX} pack_tests.cpp
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
  )
  EkatCreateUnitTest(pack_kokkos${DP_POSTFIX} pack_kokkos_tests.cpp
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
  )
endif ()

if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(pack${SP_POSTFIX} pack_tests.cpp
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
  )
  EkatCreateUnitTest(pack_kokkos${SP_POSTFIX} pack_kokkos_tests.cpp
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
  )
endif ()

# Test pack index arithmetics utils
EkatCreateUnitTest(pack_utils pack_utils_tests.cpp LIBS ekat)
