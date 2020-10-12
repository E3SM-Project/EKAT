include(EkatCreateUnitTest)

# Test utilities (f90)
set (ARRAY_IO_SRCS
  array_io_tests.cpp
  array_io_tests.f90
)
if (EKAT_TEST_DOUBLE_PRECISION)
  EkatCreateUnitTest(array_io${DP_POSTFIX} "${ARRAY_IO_SRCS}"
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_DOUBLE_PRECISION
  )
endif ()
if (EKAT_TEST_SINGLE_PRECISION)
  EkatCreateUnitTest(array_io${SP_POSTFIX} "${ARRAY_IO_SRCS}"
    LIBS ekat
    COMPILER_DEFS EKAT_TEST_SINGLE_PRECISION
  )
endif ()

# YAML parser
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.yaml ${CMAKE_CURRENT_BINARY_DIR}/input.yaml COPYONLY)
EkatCreateUnitTest(yaml_parser yaml_parser.cpp
  LIBS ekat
)
