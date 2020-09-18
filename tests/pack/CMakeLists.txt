include(EkatCreateUnitTest)

# Test packs
set (PACK_TESTS_SRCS
  pack_tests.cpp
  pack_kokkos_tests.cpp
)

if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(packs${DP_POSTFIX} "${PACK_TESTS_SRCS}"
    LIBS ekat
  )
  target_compile_definitions(packs${DP_POSTFIX} PRIVATE EKAT_TEST_DOUBLE_PRECISION)
endif ()

if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(packs${SP_POSTFIX} "${PACK_TESTS_SRCS}"
    LIBS ekat
  )
  target_compile_definitions(packs${SP_POSTFIX} PRIVATE EKAT_TEST_SINGLE_PRECISION)
endif ()

# Test pack index arithmetics utils
EkatCreateUnitTest(pack_utils pack_utils_tests.cpp LIBS ekat)
