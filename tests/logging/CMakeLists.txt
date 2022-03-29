include(EkatCreateUnitTest)

# Test basic logger capabilities
EkatCreateUnitTest(serial_file_log serial_file_log_tests.cpp LIBS ekat)

EkatCreateUnitTest(mpi_file_log_tests mpi_file_log_tests.cpp
  LIBS ekat
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS 1 ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
  PROPERTIES FAIL_REGULAR_EXPRESSION "rank1"
)

# We use the std filesystem library to check no files with a certain
# pattern were generated. To do that, we need to check if either
# <experimental/filesystem> or <filesystem> exist, preferring the former,
# since the latter is basically empty if C++17 is not enabled.
file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/check_exp_fs.cpp
  "#include <experimental/filesystem>\n"
  "int main(int,char**) { return 0; }"
  )
try_compile(EXP_FS_EXISTS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/check_exp_fs.cpp)
file (REMOVE ${CMAKE_CURRENT_BINARY_DIR}/check_exp_fs.cpp)

file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/check_fs.cpp
  "#include <filesystem>\n"
  "int main(int,char**) { return 0; }"
  )
try_compile(FS_EXISTS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/check_fs.cpp)
file (REMOVE ${CMAKE_CURRENT_BINARY_DIR}/check_fs.cpp)
if (EXP_FS_EXISTS)
  set (CXX_DEFS "HAS_EXP_FILESYSTEM")
elseif(FS_EXISTS)
  set (CXX_DEFS "HAS_FILESYSTEM")
else()
  message (FATAL_ERROR "Could not detect either <experimental/filesystem> nor <filesystem>")
endif()

# Console-only output. One logger should print "all_ranks.$size.$rank"
# for all ranks. The other should print only "root_rank.$size.0"
set (pass "all_ranks.${EKAT_TEST_MAX_RANKS}.${RANK}")
set (fail "QWERTY") # Not in the output anyways

foreach(RANK RANGE 1 ${EKAT_TEST_MAX_RANKS})
  string(APPEND pass "|all_ranks.${EKAT_TEST_MAX_RANKS}.${RANK}")
  if (RANK GREATER 1)
    string(APPEND fail "|root_rank.${EKAT_TEST_MAX_RANKS}.${RANK}")
  endif()
endforeach()

EkatCreateUnitTest(console_only_log console_only_log_tests.cpp
  LIBS ekat stdc++fs
  COMPILER_CXX_DEFS ${CXX_DEFS}
  MPI_EXEC_NAME ${EKAT_TEST_MPI_EXEC_NAME}
  MPI_RANKS ${EKAT_TEST_MAX_RANKS}
  MPI_NP_FLAG ${EKAT_TEST_MPI_NP_FLAG}
  MPI_EXTRA_ARGS "--oversubscribe"
  PROPERTIES PASS_REGULAR_EXPRESSION ${pass}
  PROPERTIES FAIL_REGULAR_EXPRESSION ${fail}
)
