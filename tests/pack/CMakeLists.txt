include(EkatCreateUnitTest)

# Test packs
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(pack${DP_POSTFIX} pack.cpp
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    LIBS ekat::Pack)
endif()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(pack${SP_POSTFIX} pack.cpp
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    LIBS ekat::Pack)
endif()

# Test pack kokkos utils
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(pack_kokkos${DP_POSTFIX} pack_kokkos_utils.cpp
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
    LIBS ekat::Pack)
endif ()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(pack_kokkos${SP_POSTFIX} pack_kokkos_utils.cpp
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
    LIBS ekat::Pack)
endif ()

# Test Unmanaged with scalar/pack views (c++)
EkatCreateUnitTest(unmanaged unmanaged.cpp
  LIBS ekat::Pack)

# Test the where construct
EkatCreateUnitTest(pack_where pack_where.cpp
  LIBS ekat::Pack)

# Test pack index arithmetics utils
EkatCreateUnitTest(pack_utils pack_utils.cpp
  LIBS ekat::Pack)
