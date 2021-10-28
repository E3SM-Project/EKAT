include(CMakeParseArguments) # Needed for backwards compatibility
include(EkatUtils) # To check macro args

# This function takes the following mandatory arguments:
#    - target_name: the name of the executable
#    - target_srcs: a list of src files for the executable
#      Note: no need to include ekat_catch_main; this macro will add it
# and the following optional arguments (to be passed as ARG_NAME "ARG_VAL")
#    - MPI_RANKS: the number of mpi ranks for the test.
#      Note: if 2 values, it's a range, if 3, it's a range plus increment. default is np=1
#    - THREADS: the number of threads for the test
#      Note: if 2 values, it's a range, if 3, it's a range plus an increment. default is 1 thread
#      Note: for each combination of valid mpi-rank and thread value, a new test will be created,
#            with suffix '_npN_omp_M', with N numver of mpi ranks, and M number of omp threads.
#    - MPI_EXEC_NAME: name of the mpi launcher (usually, mpiexe or mpirun, but may be another wrapper)
#    - MPI_NP_FLAG: the flag used to specify the number of mpi ranks (usually, -np or -n).
#                   If --map-by is used, the macro will pass `--map-by ppr:NRANKS:pe=NTHREADS` to mpiexec
#    - MPI_EXTRA_ARGS: additional args to be forwarded to the mpi launches (e.g., --map-by, --bind-to, ...)
#    - COMPILE_DEFS: a list of additional defines for the compiler
#    - COMPILER_FLAGS: a list of additional flags for the compiler
#    - LIBS: a list of libraries needed by the executable
#    - LIBS_DIRS: a list of directories to add to the linker search path
#    - LINKER_FLAGS: a list of additional flags for the linker
#    - LABELS: a set of labels to attach to the test
#    - PROPERTIES: a list of properties for ALL the tests in the threads/ranks combinations
#    - SERIAL: if this options is present, the different tests (corresponding to different
#      THREADS/RANKS combination will NOT be allowed to run concurrently (via setting of a RESOURCE_LOCK).

# Note: we hace to set this variable here, so CMAKE_CURRENT_LIST_DIR gets the
#       directory of this file. If we did it inside the function, it would get
#       the directory from where the function is called
set (CATCH_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../extern/catch2/include)

function(EkatCreateUnitTest target_name target_srcs)

  #---------------------------#
  #   Parse function inputs   #
  #---------------------------#

  set(options EXCLUDE_MAIN_CPP EXCLUDE_TEST_SESSION SERIAL THREADS_SERIAL RANKS_SERIAL PRINT_OMP_AFFINITY)
  set(oneValueArgs DEP MPI_EXEC_NAME MPI_NP_FLAG)
  set(multiValueArgs
    MPI_RANKS THREADS
    MPI_EXTRA_ARGS EXE_ARGS
    INCLUDE_DIRS
    COMPILER_DEFS
    COMPILER_C_DEFS COMPILER_CXX_DEFS COMPILER_F_DEFS
    COMPILER_FLAGS
    COMPILER_C_FLAGS COMPILER_CXX_FLAGS COMPILER_F_FLAGS
    LIBS LIBS_DIRS LINKER_FLAGS
    LABELS PROPERTIES)

  # ecut = Ekat Create Unit Test
  cmake_parse_arguments(ecut "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  CheckMacroArgs(EkatCreateUnitTest ecut "${options}" "${oneValueArgs}" "${multiValueArgs}")

  # Strip leading/trailing whitespaces from some vars, to avoid either cmake errors
  # (e.g., in target_link_libraries) or compiler errors (e.g. if COMPILER_DEFS=" ")
  string(STRIP "${ecut_LIBS}" ecut_LIBS)
  string(STRIP "${ecut_COMPILER_DEFS}" ecut_COMPILER_DEFS)
  string(STRIP "${ecut_COMPILER_C_DEFS}" ecut_COMPILER_C_DEFS)
  string(STRIP "${ecut_COMPILER_CXX_DEFS}" ecut_COMPILER_CXX_DEFS)
  string(STRIP "${ecut_COMPILER_F_DEFS}" ecut_COMPILER_F_DEFS)
  string(STRIP "${ecut_COMPILER_FLAGS}" ecut_COMPILER_FLAGS)
  string(STRIP "${ecut_COMPILER_C_FLAGS}" ecut_COMPILER_C_FLAGS)
  string(STRIP "${ecut_COMPILER_CXX_FLAGS}" ecut_COMPILER_CXX_FLAGS)
  string(STRIP "${ecut_COMPILER_F_FLAGS}" ecut_COMPILER_F_FLAGS)
  string(STRIP "${ecut_LIBS_DIRS}" ecut_LIBS_DIRS)
  string(STRIP "${ecut_INCLUDE_DIRS}" ecut_INCLUDE_DIRS)

  #---------------------------#
  #   Create the executable   #
  #---------------------------#

  # Set link directories (must be done BEFORE add_executable is called)
  # NOTE: CMake 3.15 adds 'target_link_directories', which is superior (does not pollute other targets).
  #       We should switch to that as soon as we can assume CMAKE_VERSION >= 3.15.
  if (ecut_LIBS_DIRS)
    link_directories("${ecut_LIBS_DIRS}")
  endif()
  add_executable (${target_name} ${target_srcs})

  #---------------------------#
  # Set all target properties #
  #---------------------------#

  # Include dirs
  target_include_directories(${target_name} PUBLIC
      ${CATCH_INCLUDE_DIR}
       ${CMAKE_CURRENT_SOURCE_DIR}
       ${CMAKE_CURRENT_BINARY_DIR}
       ${ecut_INCLUDE_DIRS}
  )

  # F90 output dir
  set_target_properties(${target_name} PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_modules)

  # Link flags/libs
  if (NOT ecut_EXCLUDE_MAIN_CPP)
    target_link_libraries(${target_name} PUBLIC ekat_test_main)
  endif ()
  if (NOT ecut_EXCLUDE_TEST_SESSION)
    target_link_libraries(${target_name} PUBLIC ekat_test_session)
  endif ()
  if (ecut_LIBS)
    target_link_libraries(${target_name} PUBLIC "${ecut_LIBS}")
  endif()
  if (ecut_LINKER_FLAGS)
    set_target_properties(${target_name} PROPERTIES LINK_FLAGS "${ecut_LINKER_FLAGS}")
  endif()

  # Compiler definitions
  if (ecut_COMPILER_DEFS)
    target_compile_definitions(${target_name} PUBLIC "${ecut_COMPILER_DEFS}")
  endif()
  if (ecut_COMPILER_C_DEFS)
    target_compile_definitions(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:C>:${ecut_COMPILER_C_DEFS}>)
  endif()
  if (ecut_COMPILER_CXX_DEFS)
    target_compile_definitions(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${ecut_COMPILER_CXX_DEFS}>)
  endif()
  if (ecut_COMPILER_F_DEFS)
    target_compile_definitions(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:${ecut_COMPILER_F_DEFS}>)
  endif()

  # Compiler options
  if (ecut_COMPILER_FLAGS)
    target_compile_options(${target_name} PUBLIC "${ecut_COMPILER_FLAGS}")
  endif()
  if (ecut_COMPILER_C_FLAGS)
    target_compile_options(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:C>:${ecut_COMPILER_C_FLAGS}>)
  endif()
  if (ecut_COMPILER_CXX_FLAGS)
    target_compile_options(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${ecut_COMPILER_CXX_FLAGS}>)
  endif()
  if (ecut_COMPILER_F_FLAGS)
    target_compile_options(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:${ecut_COMPILER_F_FLAGS}>)
  endif()

  #--------------------------#
  # Setup MPI/OpenMP configs #
  #--------------------------#

  list(LENGTH ecut_MPI_RANKS NUM_MPI_RANK_ARGS)
  list(LENGTH ecut_THREADS   NUM_THREAD_ARGS)

  if (NUM_MPI_RANK_ARGS GREATER 3)
    message(FATAL_ERROR "Too many mpi arguments for ${target_name}")
  endif()
  if (NUM_THREAD_ARGS GREATER 3)
    message(FATAL_ERROR "Too many thread arguments for ${target_name}")
  endif()

  set(MPI_START_RANK 1)
  set(MPI_END_RANK 1)
  set(MPI_INCREMENT 1)

  set(THREAD_START 1)
  set(THREAD_END 1)
  set(THREAD_INCREMENT 1)

  if (NUM_MPI_RANK_ARGS EQUAL 0)
  elseif(NUM_MPI_RANK_ARGS EQUAL 1)
    list(GET ecut_MPI_RANKS 0 RETURN_VAL)
    set(MPI_START_RANK ${RETURN_VAL})
    set(MPI_END_RANK ${RETURN_VAL})
  elseif(NUM_MPI_RANK_ARGS EQUAL 2)
    list(GET ecut_MPI_RANKS 0 RETURN_VAL)
    set(MPI_START_RANK ${RETURN_VAL})
    list(GET ecut_MPI_RANKS 1 RETURN_VAL)
    set(MPI_END_RANK ${RETURN_VAL})
  else()
    list(GET ecut_MPI_RANKS 0 RETURN_VAL)
    set(MPI_START_RANK ${RETURN_VAL})
    list(GET ecut_MPI_RANKS 1 RETURN_VAL)
    set(MPI_END_RANK ${RETURN_VAL})
    list(GET ecut_MPI_RANKS 2 RETURN_VAL)
    set(MPI_INCREMENT ${RETURN_VAL})
  endif()

  if (NUM_THREAD_ARGS EQUAL 0)
  elseif(NUM_THREAD_ARGS EQUAL 1)
    list(GET ecut_THREADS 0 RETURN_VAL)
    set(THREAD_START ${RETURN_VAL})
    set(THREAD_END ${RETURN_VAL})
  elseif(NUM_THREAD_ARGS EQUAL 2)
    list(GET ecut_THREADS 0 RETURN_VAL)
    set(THREAD_START ${RETURN_VAL})
    list(GET ecut_THREADS 1 RETURN_VAL)
    set(THREAD_END ${RETURN_VAL})
  else()
    list(GET ecut_THREADS 0 RETURN_VAL)
    set(THREAD_START ${RETURN_VAL})
    list(GET ecut_THREADS 1 RETURN_VAL)
    set(THREAD_END ${RETURN_VAL})
    list(GET ecut_THREADS 2 RETURN_VAL)
    set(THREAD_INCREMENT ${RETURN_VAL})
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
  if (NOT ecut_MPI_EXEC_NAME)
    if (NOT MPI_START_RANK EQUAL MPI_END_RANK)
      message (FATAL_ERROR "Error! MPI_START_RANK != MPI_END_RANK, but MPI_EXEC_NAME was not given.")
    endif()
  endif()

  #------------------------------------------------#
  # Loop over MPI/OpenMP configs, and create tests #
  #------------------------------------------------#

  # Set up launcher prefix
  set(launcher "${CMAKE_BINARY_DIR}/bin/test-launcher")
  if (ecut_PRINT_OMP_AFFINITY)
    string(APPEND launcher " -p")
  endif()
  if (EKAT_TEST_LAUNCHER_NO_BUFFER)
    string(APPEND launcher " -b")
  endif()
  string(APPEND launcher " -e")

  if (ecut_EXE_ARGS)
    set(invokeExec "./${target_name} ${ecut_EXE_ARGS}")
  else()
    set(invokeExec "./${target_name}")
  endif()

  foreach (NRANKS RANGE ${MPI_START_RANK} ${MPI_END_RANK} ${MPI_INCREMENT})
    foreach (NTHREADS RANGE ${THREAD_START} ${THREAD_END} ${THREAD_INCREMENT})
      # Create the test name
      set(FULL_TEST_NAME ${target_name}_ut_np${NRANKS}_omp${NTHREADS})

      # Setup valgrind/memcheck commmand modifications
      if (EKAT_ENABLE_VALGRIND)
        set(VALGRIND_SUP_FILE "${CMAKE_BINARY_DIR}/mpi.supp")
        set(invokeExecCurr "valgrind --error-exitcode=1 --suppressions=${VALGRIND_SUP_FILE} ${invokeExec}")
      elseif(EKAT_ENABLE_CUDA_MEMCHECK)
        set(invokeExecCurr "cuda-memcheck --error-exitcode 1 ${invokeExec}")
      else()
        set(invokeExecCurr "${invokeExec}")
      endif()

      # Prepend launcher to serial command
      set(invokeExecCurr "${launcher} ${invokeExecCurr}")

      # Create the test.
      if (ecut_MPI_EXEC_NAME)
        if (ecut_MPI_NP_FLAG STREQUAL "--map-by")
          set (RANK_MAPPING "--map-by ppr:${NRANKS}:node:pe=${NTHREADS}")
        else()
          set (RANK_MAPPING "${ecut_MPI_NP_FLAG} ${NRANKS}")
        endif()
        add_test(NAME ${FULL_TEST_NAME}
                 COMMAND sh -c "${ecut_MPI_EXEC_NAME} ${RANK_MAPPING} ${ecut_MPI_EXTRA_ARGS} ${invokeExecCurr}")
      else()
        add_test(NAME ${FULL_TEST_NAME} COMMAND sh -c "${invokeExecCurr}")
      endif()

      # Set test properties
      math(EXPR CURR_CORES "${NRANKS}*${NTHREADS}")
      set_tests_properties(${FULL_TEST_NAME} PROPERTIES ENVIRONMENT OMP_NUM_THREADS=${NTHREADS} PROCESSORS ${CURR_CORES} PROCESSOR_AFFINITY True)
      if (ecut_DEP AND NOT ecut_DEP STREQUAL "${FULL_TEST_NAME}")
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES DEPENDS ${ecut_DEP})
      endif()

      if (ecut_LABELS)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES LABELS "${ecut_LABELS}")
      endif()

      if (ecut_PROPERTIES)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES ${ecut_PROPERTIES})
      endif()

      set(RES_GROUPS "")
      foreach (rank RANGE 1 ${CURR_CORES})
        list (APPEND RES_GROUPS "devices:1")
      endforeach()
      string(REPLACE ";" "," RES_GROUPS "${RES_GROUPS}")

      set_property(TEST ${FULL_TEST_NAME} PROPERTY RESOURCE_GROUPS "${RES_GROUPS}")
    endforeach()
  endforeach()

  if (ecut_SERIAL)
    # All tests run serially
    set (tests_names)
    foreach (NRANKS RANGE ${MPI_START_RANK} ${MPI_END_RANK} ${MPI_INCREMENT})
      foreach (NTHREADS RANGE ${THREAD_START} ${THREAD_END} ${THREAD_INCREMENT})
        # Create the test
        set(FULL_TEST_NAME ${target_name}_ut_np${NRANKS}_omp${NTHREADS})
        list(APPEND tests_names ${FULL_TEST_NAME})
      endforeach ()
    endforeach()
    set_tests_properties (${tests_names} PROPERTIES RESOURCE_LOCK ${target_name}_serial)
  endif ()

endfunction(EkatCreateUnitTest)
