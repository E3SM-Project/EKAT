include(CMakeParseArguments) # Needed for backwards compatibility
include(EkatUtils) # To check macro args

# This function creates an executable, and a bunch of tests,
# all running said executable, for a variety of MPI/Threads
# combinations.
#
# This function takes the following mandatory arguments:
#    - target_name: the name of the executable
#    - target_srcs: a list of src files for the executable
#      Note: no need to include ekat_catch_main; this macro will add it
#
# The following are optional arguments (to be passed as ARG_NAME "ARG_VAL")
#    - MPI_RANKS: the number of mpi ranks for the test.
#      Note: if 2 values, it's a range, if 3, it's a range plus increment. default is np=1
#    - THREADS: the number of threads for the test
#      Note: if 2 values, it's a range, if 3, it's a range plus an increment. default is 1 thread
#      Note: for each combination of valid mpi-rank and thread value, a new test will be created,
#            with suffix '_npN_omp_M', with N numver of mpi ranks, and M number of omp threads.
#    - MPI_EXEC_NAME: name of the mpi launcher (usually, mpiexe or mpirun, but may be another wrapper)
#    - MPI_NP_FLAG: the flag used to specify the number of mpi ranks (usually, -np or -n)
#    - MPI_EXTRA_ARGS: additional args to be forwarded to the mpi launches (e.g., --map-by, --bind-to, ...)
#    - COMPILE_DEFS: a list of additional defines for the compiler
#    - COMPILER_FLAGS: a list of additional flags for the compiler
#    - LIBS: a list of libraries needed by the executable
#    - LIBS_DIRS: a list of directories to add to the linker search path
#    - LINKER_FLAGS: a list of additional flags for the linker
#    - DEPS: a set of tests that this set of tests depends on
#    - LABELS: a set of labels to attach to the test

# Note: we hace to set this variable here, so CMAKE_CURRENT_LIST_DIR gets the
#       directory of this file. If we did it inside the function, it would get
#       the directory from where the function is called
set (CATCH_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../extern/catch2/include)

function(EkatCreateUnitTest target_name target_srcs)

  #---------------------------#
  #   Parse function inputs   #
  #---------------------------#

  set(options EXCLUDE_MAIN_CPP EXCLUDE_TEST_SESSION)
  set(oneValueArgs MPI_EXEC_NAME MPI_NP_FLAG)
  set(multiValueArgs
    MPI_RANKS THREADS
    MPI_EXTRA_ARGS EXE_ARGS 
    INCLUDE_DIRS
    COMPILER_DEFS
    COMPILER_C_DEFS COMPILER_CXX_DEFS COMPILER_F_DEFS
    COMPILER_FLAGS
    COMPILER_C_FLAGS COMPILER_CXX_FLAGS COMPILER_F_FLAGS
    LIBS LIBS_DIRS LINKER_FLAGS
    DEPS LABELS)

  # ecut = Ekat Create Unit Test
  cmake_parse_arguments(ecut "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  CheckMacroArgs(EkatCreateUnitTest ecut "${options}" "${oneValueArgs}" "${multiValueArgs}")

  # Strip leading/trailing whitespaces from some vars, to avoid either cmake errors
  # (e.g., in target_link_libraries) or compiler errors (e.g. if COMPILER_DEFS=" ")
  string(STRIP "${ecut_DEPS}" ecut_DEPS)
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
    message(FATAL_ERROR "Too many mpi arguments for ${target_name}: ${ecut_MPI_RANKS}")
  endif()
  if (NUM_THREAD_ARGS GREATER 3)
    message(FATAL_ERROR "Too many thread arguments for ${target_name}: ${ecut_THREADS} ")
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

  # Check both, in case user has negative increment
  if (MPI_END_RANK GREATER 1 OR MPI_START_RANK GREATER 1)
    if ("${ecut_MPI_EXEC_NAME}" STREQUAL "")
      set (ecut_MPI_EXEC_NAME "mpiexec")
    endif()
    if ("${ecut_MPI_NP_FLAG}" STREQUAL "")
      set (ecut_MPI_NP_FLAG "-n")
    endif()
  endif()

  #------------------------------------------------#
  # Loop over MPI/OpenMP configs, and create tests #
  #------------------------------------------------#

  if (ecut_EXE_ARGS)
    set(invokeExec "./${target_name} ${ecut_EXE_ARGS}")
  else()
    set(invokeExec "./${target_name}")
  endif()

  foreach (NRANKS RANGE ${MPI_START_RANK} ${MPI_END_RANK} ${MPI_INCREMENT})
    foreach (NTHREADS RANGE ${THREAD_START} ${THREAD_END} ${THREAD_INCREMENT})
      # Create the test
      set(FULL_TEST_NAME ${target_name}_ut_np${NRANKS}_omp${NTHREADS})
      if (${NRANKS} GREATER 1)
        add_test(NAME ${FULL_TEST_NAME}
                 COMMAND sh -c "${ecut_MPI_EXEC_NAME} ${ecut_MPI_NP_FLAG} ${NRANKS} ${ecut_MPI_EXTRA_ARGS} ${invokeExec}")
      else()
        add_test(NAME ${FULL_TEST_NAME}
                 COMMAND sh -c "${invokeExec}")
      endif()
      math(EXPR CURR_CORES "${NRANKS}*${NTHREADS}")
      set_tests_properties(${FULL_TEST_NAME} PROPERTIES
          ENVIRONMENT "OMP_NUM_THREADS=${NTHREADS};OMP_PROC_BIND=spread;OMP_PLACES=threads"
          PROCESSORS ${CURR_CORES}
          PROCESSOR_AFFINITY True)
 
      if (ecut_DEPS)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES DEPENDS "${ecut_DEPS}")
      endif()

      if (ecut_LABELS)
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES LABELS "${ecut_LABELS}")
      endif()

      # Note: with cmake 3.15, you could use string(REPEAT ...) instead of foreach
      set (RES_GROUPS "devices:1")
      if (${CURR_CORES} GREATER 1)
        foreach (core RANGE 2 ${CURR_CORES} 1)
          set (RES_GROUPS "${RES_GROUPS},devices:1")
        endforeach()
      endif ()
      set_property(TEST ${FULL_TEST_NAME} PROPERTY RESOURCE_GROUPS "${RES_GROUPS}")
    endforeach()
  endforeach()

endfunction(EkatCreateUnitTest)
