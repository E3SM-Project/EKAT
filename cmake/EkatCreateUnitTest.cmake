include(CMakeParseArguments) # Needed for backwards compatibility
include(EkatUtils) # To check macro args

# Note: we have to set this variable here, so CMAKE_CURRENT_LIST_DIR gets the
#       directory of this file. If we did it inside the function, it would get
#       the directory from where the function is called
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../extern/Catch2/single_include)

set(CUT_EXEC_OPTIONS EXCLUDE_MAIN_CPP USER_DEFINED_TEST_SESSION)
set(CUT_EXEC_1V_ARGS)
set(CUT_EXEC_MV_ARGS
  INCLUDE_DIRS
  COMPILER_DEFS
  COMPILER_C_DEFS COMPILER_CXX_DEFS COMPILER_F_DEFS
  COMPILER_FLAGS
  COMPILER_C_FLAGS COMPILER_CXX_FLAGS COMPILER_F_FLAGS
  LIBS LIBS_DIRS LINKER_FLAGS)

# NOTE: for FIXTURES properties, the version without suffix denotes a property that has
#       to be set on all RANKS/THREADS combinations *with the same name*,
#       while the INDIVIDUAL suffix denotes a property that has to be set on
#       a per-test basis. In the latter case, the actual FIXTURES name set
#       on each test will have the "_npN_ompM" suffix attached.
#       This can help if the tests setup some fixture (e.g., an output file)
#       that a downstream test checks. Without this options, *all* ranks/threads
#       combinations would have to complete (successfully) before the single
#       downstream test can check their results. With this options, we can have
#       one individual downstream test for each rank/thread combination,
#       allowing the user to debug possible errors with a test more quickly.
set(CUT_TEST_OPTIONS SERIAL THREADS_SERIAL RANKS_SERIAL PRINT_OMP_AFFINITY WILL_FAIL)
set(CUT_TEST_MV_ARGS DEP EXE_ARGS MPI_RANKS THREADS LABELS PROPERTIES
    FIXTURES_SETUP FIXTURES_REQUIRED FIXTURES_CLEANUP
    FIXTURES_SETUP_INDIVIDUAL FIXTURES_REQUIRED_INDIVIDUAL FIXTURES_CLEANUP_INDIVIDUAL)

# This function takes the following mandatory arguments:
#    - exec_name: the name of the test executable that will be created.
#    - exec_srcs: a list of src files for the executable.
#      Note: no need to include ekat_catch_main.cpp; this macro will add it (if needed).
# The following optional arguments can be passed as ARG_NAME "ARG_VAL":
#    - INCLUDE_DIRS: a list of directories to add to the include search path
#    - COMPILE_[C_|CXX_|F_]DEFS: a list of additional (possibly language-specific) defines for the compiler
#    - COMPILER_[C_|CXX_|F_]FLAGS: a list of additional flags (possibly language-specific) for the compiler
#    - LIBS: a list of libraries needed by the executable (i.e., libs/targets to link against)
#    - LIBS_DIRS: a list of directories to add to the linker search path
#    - LINKER_FLAGS: a list of additional flags for the linker
function(EkatCreateUnitTestExec exec_name exec_srcs)
  #---------------------------#
  #   Parse function inputs   #
  #---------------------------#

  # ecute = Ekat Create Unit Test Exec
  cmake_parse_arguments(ecute "${CUT_EXEC_OPTIONS}" "${CUT_EXEC_1V_ARGS}" "${CUT_EXEC_MV_ARGS}" ${ARGN})
  CheckMacroArgs(EkatCreateUnitTestExec ecute "${CUT_EXEC_OPTIONS}" "${CUT_EXEC_1V_ARGS}" "${CUT_EXEC_MV_ARGS}")

  # Strip leading/trailing whitespaces from args to avoid either cmake errors
  # (e.g., in target_link_libraries) or compiler errors (e.g. if COMPILER_DEFS=" ")
  foreach(item IN LISTS CUT_EXEC_1V_ARGS CUT_EXEC_MV_ARGS)
    string(STRIP "${ecute_${item}}" ecute_${item})
  endforeach()

  #-------------------------------------------------#
  #   Create the executable and set its properties  #
  #-------------------------------------------------#

  # Set link directories (must be done BEFORE add_executable is called)
  # NOTE: CMake 3.15 adds 'target_link_directories', which is superior (does not pollute other targets).
  #       We should switch to that as soon as we can assume CMAKE_VERSION >= 3.15.
  set(target_name ${exec_name})
  if (ecute_LIBS_DIRS)
    link_directories("${ecute_LIBS_DIRS}")
  endif()
  add_executable(${target_name} ${exec_srcs})

  #---------------------------#
  # Set all target properties #
  #---------------------------#

  # Include dirs
  target_include_directories(${exec_name} PUBLIC
    ${CATCH_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ecute_INCLUDE_DIRS}
    )

  # Check if we need a Fortran modules folder
  foreach (file ${exec_srcs})
    get_filename_component(ext ${file} EXT)
    string(REGEX REPLACE "^\\." "" ext ${ext})

    if (ext IN_LIST CMAKE_Fortran_SOURCE_FILE_EXTENSIONS)
      set_target_properties(${target_name} PROPERTIES
        Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_modules)
      break()
    endif()
  endforeach()

  # Link flags/libs
  if (NOT ecute_EXCLUDE_MAIN_CPP)
    target_link_libraries(${target_name} PUBLIC ekat::CatchMain)
  endif ()
  if (NOT ecute_USER_DEFINED_TEST_SESSION)
    target_link_libraries(${target_name} PUBLIC ekat::DefaultTestSession)
  endif ()
  if (ecute_LIBS)
    target_link_libraries(${target_name} PUBLIC "${ecute_LIBS}")
  endif()
  if (ecute_LINKER_FLAGS)
    set_target_properties(${target_name} PROPERTIES LINK_FLAGS "${ecute_LINKER_FLAGS}")
  endif()

  # Compiler definitions
  if (ecute_COMPILER_DEFS)
    target_compile_definitions(${target_name} PUBLIC "${ecute_COMPILER_DEFS}")
  endif()
  if (ecute_COMPILER_C_DEFS)
    target_compile_definitions(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:C>:${ecute_COMPILER_C_DEFS}>)
  endif()
  if (ecute_COMPILER_CXX_DEFS)
    target_compile_definitions(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${ecute_COMPILER_CXX_DEFS}>)
  endif()
  if (ecute_COMPILER_F_DEFS)
    target_compile_definitions(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:${ecute_COMPILER_F_DEFS}>)
  endif()

  # Compiler options
  if (ecute_COMPILER_FLAGS)
    target_compile_options(${target_name} PUBLIC "${ecute_COMPILER_FLAGS}")
  endif()
  if (ecute_COMPILER_C_FLAGS)
    target_compile_options(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:C>:${ecute_COMPILER_C_FLAGS}>)
  endif()
  if (ecute_COMPILER_CXX_FLAGS)
    target_compile_options(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${ecute_COMPILER_CXX_FLAGS}>)
  endif()
  if (ecute_COMPILER_F_FLAGS)
    target_compile_options(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:${ecute_COMPILER_F_FLAGS}>)
  endif()

endfunction(EkatCreateUnitTestExec)

# Given an executable, create a suite of unit tests.
# This function takes the following mandatory argument:
#    - test_exec: the executable to be used for all the unit tests. The name of each individual unit test
#      will be ${test_exec}_np{R}_omp{T}, where R and T are the number of MPI ranks and OMP threads
#      used for the test, respectively. If R can only take the value '1' the string _np{R} is omitted
#      (and similarly for _omp{T}).
# The following optional arguments can be passed as ARG_NAME "ARG_VAL"
#    - MPI_RANKS: the number of mpi ranks for the test.
#      Note: if 2 values, it's a range, if 3, it's a range plus increment. default is np=1
#    - THREADS: the number of threads for the test
#      Note: if 2 values, it's a range, if 3, it's a range plus an increment. default is 1 thread
#      Note: for each combination of valid mpi-rank and thread value, a new test will be created,
#            with suffix '_npN_omp_M', with N numver of mpi ranks, and M number of omp threads.
#    - TEST_EXE: name of a pre-existing test executable. Use this to reuse
#    - LABELS: a set of labels to attach to the test
#    - PROPERTIES: a list of properties for ALL the tests in the threads/ranks combinations
#    - SERIAL: if this options is present, the different tests (corresponding to different
#      THREADS/RANKS combination will NOT be allowed to run concurrently (via setting of a RESOURCE_LOCK).
function(EkatCreateUnitTestFromExec test_name test_exec)

  #---------------------------#
  #   Parse function inputs   #
  #---------------------------#

  # ecutfe = Ekat Create Unit Test From Exec
  cmake_parse_arguments(ecutfe "${CUT_TEST_OPTIONS}" "${CUT_TEST_1V_ARGS}" "${CUT_TEST_MV_ARGS}" ${ARGN})
  CheckMacroArgs(EkatCreateUnitTestFromExec ecutfe "${CUT_TEST_OPTIONS}" "${CUT_TEST_1V_ARGS}" "${CUT_TEST_MV_ARGS}" ${ARGN})

  # For catch2-based tests only, pass option to remove colours (doesn't play well with CTest log files)
  get_target_property(LINKED_LIBS ${test_exec} LINK_LIBRARIES)
  if (ekat_catchmain IN_LIST LINKED_LIBS)
    if (NOT ecutfe_EXE_ARGS OR NOT "--use-colour no" IN_LIST ecutfe_EXE_ARGS)
      list(PREPEND ecutfe_EXE_ARGS "--use-colour no")
    endif()
  endif()

  #--------------------------#
  # Setup MPI/OpenMP configs #
  #--------------------------#

  list(LENGTH ecutfe_MPI_RANKS NUM_MPI_RANK_ARGS)
  list(LENGTH ecutfe_THREADS   NUM_THREAD_ARGS)

  if (NUM_MPI_RANK_ARGS GREATER 3)
    message(FATAL_ERROR "Too many mpi arguments for test ${test_name}: ${ecutfe_MPI_RANKS}")
  endif()
  if (NUM_THREAD_ARGS GREATER 3)
    message(FATAL_ERROR "Too many thread arguments for test ${test_name}: ${ecutfe_THREADS}")
  endif()

  set(MPI_START_RANK 1)
  set(MPI_END_RANK 1)
  set(MPI_INCREMENT 1)

  set(THREAD_START 1)
  set(THREAD_END 1)
  set(THREAD_INCREMENT 1)

  if (NUM_MPI_RANK_ARGS EQUAL 0)
  elseif(NUM_MPI_RANK_ARGS EQUAL 1)
    list(GET ecutfe_MPI_RANKS 0 RETURN_VAL)
    set(MPI_START_RANK ${RETURN_VAL})
    set(MPI_END_RANK ${RETURN_VAL})
    if (MPI_START_RANK GREATER 1)
      set (_ecutfe_ADD_RANK TRUE)
    endif()
  elseif(NUM_MPI_RANK_ARGS EQUAL 2)
    list(GET ecutfe_MPI_RANKS 0 RETURN_VAL)
    set(MPI_START_RANK ${RETURN_VAL})
    list(GET ecutfe_MPI_RANKS 1 RETURN_VAL)
    set(MPI_END_RANK ${RETURN_VAL})
    set (_ecutfe_ADD_RANK TRUE)
  else()
    list(GET ecutfe_MPI_RANKS 0 RETURN_VAL)
    set(MPI_START_RANK ${RETURN_VAL})
    list(GET ecutfe_MPI_RANKS 1 RETURN_VAL)
    set(MPI_END_RANK ${RETURN_VAL})
    list(GET ecutfe_MPI_RANKS 2 RETURN_VAL)
    set(MPI_INCREMENT ${RETURN_VAL})
    set (_ecutfe_ADD_RANK TRUE)
  endif()

  if (NUM_THREAD_ARGS EQUAL 0)
  elseif(NUM_THREAD_ARGS EQUAL 1)
    list(GET ecutfe_THREADS 0 RETURN_VAL)
    set(THREAD_START ${RETURN_VAL})
    set(THREAD_END ${RETURN_VAL})
    if (THREAD_START GREATER 1)
      set (_ecutfe_ADD_OMP TRUE)
    endif()
  elseif(NUM_THREAD_ARGS EQUAL 2)
    list(GET ecutfe_THREADS 0 RETURN_VAL)
    set(THREAD_START ${RETURN_VAL})
    list(GET ecutfe_THREADS 1 RETURN_VAL)
    set(THREAD_END ${RETURN_VAL})
    set (_ecutfe_ADD_OMP TRUE)
  else()
    list(GET ecutfe_THREADS 0 RETURN_VAL)
    set(THREAD_START ${RETURN_VAL})
    list(GET ecutfe_THREADS 1 RETURN_VAL)
    set(THREAD_END ${RETURN_VAL})
    list(GET ecutfe_THREADS 2 RETURN_VAL)
    set(THREAD_INCREMENT ${RETURN_VAL})
    set (_ecutfe_ADD_OMP TRUE)
  endif()

  if (NOT MPI_START_RANK GREATER 0)
    message (FATAL_ERROR "Error! MPI_START_RANK is <=0.")
  endif()
  if (NOT MPI_END_RANK GREATER 0)
    message (FATAL_ERROR "Error! MPI_END_RANK is <=0.")
  endif()
  if (MPI_INCREMENT GREATER 0 AND MPI_START_RANK GREATER MPI_END_RANK)
    message (FATAL_ERROR "Error! MPI_START_RANK > MPI_END_RANK, but the increment is positive.")
  endif()
  if (MPI_INCREMENT LESS 0 AND MPI_START_RANK LESS MPI_END_RANK)
    message (FATAL_ERROR "Error! MPI_START_RANK < MPI_END_RANK, but the increment is negative.")
  endif()
  if (NOT THREAD_START GREATER 0)
    message (FATAL_ERROR "Error! THREAD_START is <=0.")
  endif()
  if (NOT THREAD_END GREATER 0)
    message (FATAL_ERROR "Error! THREAD_END is <=0.")
  endif()
  if (THREAD_INCREMENT GREATER 0 AND THREAD_START GREATER THREAD_END)
    message (FATAL_ERROR "Error! THREAD_START > THREAD_END, but the increment is positive.")
  endif()
  if (THREAD_INCREMENT LESS 0 AND THREAD_START LESS THREAD_END)
    message (FATAL_ERROR "Error! THREAD_START < THREAD_END, but the increment is negative.")
  endif()

  # If MPI_EXEC_NAME wasn't given, make sure we don't need more than one proc.
  if (NOT EKAT_MPIRUN_EXE)
    if (MPI_END_RANK GREATER 1)
      message (FATAL_ERROR "Error! MPI_END_RANK > 1, but EKAT_MPIRUN_EXE is not known.")
    endif()
  endif()

  #------------------------------------------------#
  # Loop over MPI/OpenMP configs, and create tests #
  #------------------------------------------------#

  # Set up launcher prefix
  set(launcher "${CMAKE_BINARY_DIR}/bin/test-launcher")
  if (ecutfe_PRINT_OMP_AFFINITY)
    string(APPEND launcher " -p")
  endif()
  if (EKAT_TEST_LAUNCHER_BUFFER)
    string(APPEND launcher " -b")
  endif()
  string(APPEND launcher " --")

  if (ecutfe_EXE_ARGS)
    string (REPLACE ";" " " ecutfe_EXE_ARGS_STR "${ecutfe_EXE_ARGS}")
    set(invokeExec "./${test_exec} ${ecutfe_EXE_ARGS_STR}")
  else()
    set(invokeExec "./${test_exec}")
  endif()

  foreach (NRANKS RANGE ${MPI_START_RANK} ${MPI_END_RANK} ${MPI_INCREMENT})
    foreach (NTHREADS RANGE ${THREAD_START} ${THREAD_END} ${THREAD_INCREMENT})
      # Create the test name
      set(FULL_TEST_NAME ${test_name})
      if (_ecutfe_ADD_RANK)
        string (APPEND FULL_TEST_NAME "_np${NRANKS}")
      endif()
      if (_ecutfe_ADD_OMP)
        string (APPEND FULL_TEST_NAME "_omp${NTHREADS}")
      endif()
      set (rank_thread_suffix "_np${NRANKS}_omp${NTHREADS}")

      # Setup valgrind/memcheck commmand modifications
      if (EKAT_ENABLE_VALGRIND)
        if (NOT EKAT_VALGRIND_SUPPRESSION_FILE)
          message(FATAL_ERROR "Valgrind is enabled but EKAT_VALGRIND_SUPPRESSION_FILE has not been set!")
        endif()
        set(invokeExecCurr "valgrind --error-exitcode=1 --suppressions=${EKAT_VALGRIND_SUPPRESSION_FILE} ${invokeExec}")
      elseif(EKAT_ENABLE_CUDA_MEMCHECK)
        set(invokeExecCurr "cuda-memcheck --error-exitcode 1 ${invokeExec}")
      elseif(EKAT_ENABLE_COMPUTE_SANITIZER)
        set(invokeExecCurr "compute-sanitizer --error-exitcode 1 ${EKAT_COMPUTE_SANITIZER_OPTIONS} ${invokeExec}")
      else()
        set(invokeExecCurr "${invokeExec}")
      endif()

      # Prepend launcher to serial command
      set(invokeExecCurr "${launcher} ${invokeExecCurr}")

      # Create the test. Run with MPI if possible.
      if (EKAT_MPIRUN_EXE)
        if (EKAT_MPI_NP_FLAG STREQUAL "--map-by")
          set (RANK_MAPPING "--map-by ppr:${NRANKS}:node:pe=${NTHREADS}")
        else()
          set (RANK_MAPPING "${EKAT_MPI_NP_FLAG} ${NRANKS}")
        endif()
        if (EKAT_MPI_THREAD_FLAG)
          string(APPEND RANK_MAPPING " ${EKAT_MPI_THREAD_FLAG} ${NTHREADS}")
        endif()
        set(FULL_TEST_CMD "${EKAT_MPIRUN_EXE} ${RANK_MAPPING} ${EKAT_MPI_EXTRA_ARGS} ${invokeExecCurr}")
      else()
        set(FULL_TEST_CMD "${invokeExecCurr}")
      endif()

      add_test(NAME ${FULL_TEST_NAME} COMMAND sh -c "${FULL_TEST_CMD}")

      # Set test properties
      math(EXPR CURR_CORES "${NRANKS}*${NTHREADS}")
      set_tests_properties(${FULL_TEST_NAME} PROPERTIES ENVIRONMENT OMP_NUM_THREADS=${NTHREADS} PROCESSORS ${CURR_CORES} PROCESSOR_AFFINITY True FULL_TEST_COMMAND "${FULL_TEST_CMD}")
      if (ecutfe_DEP AND NOT ecutfe_DEP STREQUAL "${FULL_TEST_NAME}")
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES DEPENDS ${ecutfe_DEP})
      endif()

      if (ecutfe_LABELS)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES LABELS "${ecutfe_LABELS}")
      endif()

      if (ecutfe_FIXTURES_SETUP)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES FIXTURES_SETUP "${ecutfe_FIXTURES_SETUP}")
      endif()

      if (ecutfe_FIXTURES_REQUIRED)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES FIXTURES_REQUIRED "${ecutfe_FIXTURES_REQUIRED}")
      endif()

      if (ecutfe_FIXTURES_CLEANUP)
        # We cannot have multiple rank/thread combinations for FIXTURE_CLEANUP
        if ((NOT MPI_START_RANK EQUAL MPI_END_RANK) OR
            (NOT THREAD_START EQUAL THREAD_END))
          message (FATAL_ERROR
            "Error! FIXTURE_CLEANUP property set for test with multiple rank/thread combinations")
        endif()
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES FIXTURES_CLEANUP "${ecutfe_FIXTURES_CLEANUP}")
      endif()

      if (ecutfe_FIXTURES_SETUP_INDIVIDUAL)
        foreach(item IN LISTS ecutfe_FIXTURES_SETUP_INDIVIDUAL)
          set_property(TEST ${FULL_TEST_NAME} APPEND PROPERTY FIXTURES_SETUP "${item}${rank_thread_suffix}")
        endforeach()
      endif()

      if (ecutfe_FIXTURES_REQUIRED_INDIVIDUAL)
        foreach(item IN LISTS ecutfe_FIXTURES_REQUIRED_INDIVIDUAL)
          set_property(TEST ${FULL_TEST_NAME} APPEND PROPERTY FIXTURES_REQUIRED "${item}${rank_thread_suffix}")
        endforeach()
      endif()

      if (ecutfe_FIXTURES_CLEANUP_INDIVIDUAL)
        foreach(item IN LISTS ecutfe_FIXTURES_CLEANUP_INDIVIDUAL)
          set_property(TEST ${FULL_TEST_NAME} APPEND PROPERTY FIXTURES_CLEANUP "${item}${rank_thread_suffix}")
        endforeach()
      endif()

      if (ecutfe_WILL_FAIL)
        set_property(TEST ${FULL_TEST_NAME} PROPERTY WILL_FAIL TRUE)
      endif()

      if (ecutfe_PROPERTIES)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES ${ecutfe_PROPERTIES})
      endif()

      set(RES_GROUPS "")
      foreach (rank RANGE 1 ${CURR_CORES})
        list (APPEND RES_GROUPS "devices:1")
      endforeach()
      string(REPLACE ";" "," RES_GROUPS "${RES_GROUPS}")

      set_property(TEST ${FULL_TEST_NAME} PROPERTY RESOURCE_GROUPS "${RES_GROUPS}")
    endforeach()
  endforeach()

  if (ecutfe_SERIAL)
    # All tests run serially
    set (tests_names)
    foreach (NRANKS RANGE ${MPI_START_RANK} ${MPI_END_RANK} ${MPI_INCREMENT})
      foreach (NTHREADS RANGE ${THREAD_START} ${THREAD_END} ${THREAD_INCREMENT})
        # Create the test
        set(FULL_TEST_NAME ${test_name})
        if (_ecutfe_ADD_RANK)
          string (APPEND FULL_TEST_NAME "_np${NRANKS}")
        endif()
        if (_ecutfe_ADD_OMP)
          string (APPEND FULL_TEST_NAME "_omp${NTHREADS}")
        endif()
        list(APPEND tests_names ${FULL_TEST_NAME})
      endforeach ()
    endforeach()
    set_tests_properties (${tests_names} PROPERTIES RESOURCE_LOCK ${test_name}_serial)
  endif ()

endfunction(EkatCreateUnitTestFromExec)

# This function combines the two above, to create an executable, and use it
# to create a suite of unit tests. The optional arguments are the union of
# the optional arguments of the two functions above
function(EkatCreateUnitTest test_name test_srcs)
  set(options ${CUT_EXEC_OPTIONS} ${CUT_TEST_OPTIONS})
  set(oneValueArgs ${CUT_EXEC_1V_ARGS} ${CUT_TEST_1V_ARGS})
  set(multiValueArgs ${CUT_EXEC_MV_ARGS} ${CUT_TEST_MV_ARGS})

  # ecut = Ekat Create Unit Test
  cmake_parse_arguments(ecut "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  CheckMacroArgs(EkatCreateUnitTest ecut "${options}" "${oneValueArgs}" "${multiValueArgs}")

  #------------------------------#
  #      Create Exec Phase       #
  #------------------------------#

  separate_cut_arguments(ecut "${CUT_EXEC_OPTIONS}" "${CUT_EXEC_1V_ARGS}" "${CUT_EXEC_MV_ARGS}" options_ExecPhase)
  EkatCreateUnitTestExec("${test_name}" "${test_srcs}" ${options_ExecPhase})

  #------------------------------#
  #      Create Tests Phase      #
  #------------------------------#

  separate_cut_arguments(ecut "${CUT_TEST_OPTIONS}" "${CUT_TEST_1V_ARGS}" "${CUT_TEST_MV_ARGS}" options_TestPhase)

  EkatCreateUnitTestFromExec("${test_name}" "${test_name}" ${options_TestPhase})

endfunction(EkatCreateUnitTest)
