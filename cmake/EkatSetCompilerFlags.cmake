##############################################################################
# Compiler specific options
##############################################################################

# Small function to set the compiler flag '-fp model' given the name of the model
# Note: this is an Intel-only flag
function (EKAT_set_fpmodel_flags fpmodel_string flags)
  string(TOLOWER "${fpmodel_string}" fpmodel_string_lower)
  if (("${fpmodel_string_lower}" STREQUAL "precise") OR
      ("${fpmodel_string_lower}" STREQUAL "strict") OR
      ("${fpmodel_string_lower}" STREQUAL "fast") OR
      ("${fpmodel_string_lower}" STREQUAL "fast=1") OR
      ("${fpmodel_string_lower}" STREQUAL "fast=2"))
    if (CMAKE_Fortran_COMPILER_ID STREQUAL Intel)
      set (${flags} "-fp-model ${fpmodel_string_lower}" PARENT_SCOPE)
    elseif (CMAKE_Fortran_COMPILER_ID STREQUAL GNU)
      if ("${fpmodel_string_lower}" STREQUAL "strict")
        set (${flags} "-ffp-contract=off" PARENT_SCOPE)
      endif ()
    endif ()
  elseif ("${fpmodel_string_lower}" STREQUAL "")
    set (${flags} "" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "FP_MODEL_FLAG string '${fpmodel_string}' is not recognized.")
  endif()
endfunction()

macro (SetCompilerFlags)
  set (FP_MODEL_FLAG "")
  set (UT_FP_MODEL_FLAG "")
  if (DEFINED ${PROJECT_NAME}_FPMODEL)
    EKAT_set_fpmodel_flags("${${PROJECT_NAME}_FPMODEL}" FP_MODEL_FLAG)
    string(TOLOWER "${${PROJECT_NAME}_FPMODEL}" fpmodel_string_lower)
    if (fpmodel_string_lower STREQUAL "strict")
      set (${PROJECT_NAME}_STRICT_FP TRUE PARENT_SCOPE BOOL)
    endif()
  elseif (CMAKE_Fortran_COMPILER_ID STREQUAL Intel)
    set(${FP_MODEL_FLAG} "-fp-model precise")
    set(${UT_FP_MODEL_FLAG} "-fp-model precise")
  endif ()

  # enable all warning but disable vectorization remarks like "remark: simd loop has only one iteration"
  # since we would get hit with 1000's of those anytime we set packsize to 1.
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  if (CMAKE_Fortran_COMPILER_ID STREQUAL Intel)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -diag-disable=remark")
    set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -warn all -diag-disable=remark -fpscomp logicals")
  else()
    set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall")
  endif()

  # Language-specific flags to be used regardless of build type.
  if (DEFINED BASE_FFLAGS)
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${BASE_FFLAGS}")
  else ()
    if (CMAKE_Fortran_COMPILER_ID STREQUAL GNU)
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -ffree-line-length-none ${FP_MODEL_FLAG}")
      add_definitions(-DCPRGNU)
    elseif (CMAKE_Fortran_COMPILER_ID STREQUAL Intel)
      add_definitions(-DCPRINTEL)
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -assume byterecl")
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${FP_MODEL_FLAG}")
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -ftz")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FP_MODEL_FLAG}")
    endif ()
  endif ()

  if (DEFINED BASE_CPPFLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BASE_CPPFLAGS}")
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FP_MODEL_FLAG}")
  endif ()

  # C++ Flags

  if (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -restrict")
  endif()

  STRING(TOUPPER "${PERFORMANCE_PROFILE}" PERF_PROF_UPPER)
  if ("${PERF_PROF_UPPER}" STREQUAL "VTUNE")
    add_definitions(-DVTUNE_PROFILE)
  elseif ("${PERF_PROF_UPPER}" STREQUAL "CUDA")
    add_definitions(-DCUDA_PROFILE)
  elseif ("${PERF_PROF_UPPER}" STREQUAL "GPROF")
    add_definitions(-DGPROF_PROFILE -pg)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  endif ()

  set (EKAT_COVERAGE_FLAGS "--coverage")

  if (EKAT_ENABLE_COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EKAT_COVERAGE_FLAGS}")
  endif()

  # Handle Cuda.
  find_package(CUDA QUIET)
  if (CUDA_FOUND)
    # We found cuda, but we may be only interested in running on host.
    # Check if the compiler is not nvcc; if not, do not add cuda support
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} "--nvcc-wrapper-show"
      RESULT_VARIABLE WRAPS_NVCC
      OUTPUT_VARIABLE WRAPS_NVCC_OUT1
      ERROR_QUIET)

    # Need to check OMPI_CXX/MPICH_CXX (if user is using mpicxx)
    set (mpi_distro_name)
    GetMpiDistributionName(mpi_distro_name)
    if (NOT "${mpi_distro_name}" STREQUAL "unknown")
      set (mpi_cxx_backend_var_name)
      SetMpiCxxBackendCompilerVarName(mpi_cxx_backend_var_name)
      if (DEFINED ENV{${mpi_cxx_backend_var_name}})
        execute_process(COMMAND $ENV{${mpi_cxx_backend_var_name}} "--nvcc-wrapper-show"
          RESULT_VARIABLE WRAPS_NVCC
          OUTPUT_VARIABLE WRAPS_NVCC_OUT2
          ERROR_QUIET)
      endif()
      unset (mpi_cxx_backend_var_name)
    endif()

    string (FIND "${WRAPS_NVCC_OUT1} ${WRAPS_NVCC_OUT2}" "nvcc" pos)
    if (${pos} GREATER -1)
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --expt-extended-lambda")
      # Turn off fused multiply add for debug so we can stay BFB with host
      set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --fmad=false")
      message (STATUS "Cuda enabled!")
    else ()
      message (STATUS "Cuda was found, but the C++ compiler is not nvcc_wrapper, so building without Cuda support.")
    endif ()
  endif ()

  ##############################################################################
  # Optimization flags
  # If OPT_FLAGS is set (to non-empty string), append it to Fortran/C/CXX release flags.
  # Otherwise, for LANG=F,C,CXX, if DEBUG_<LANG>FLAGS is set (to non-empty string),
  # append it to <LANG> debug flags, otherwise set some defaults.
  ##############################################################################
  if (OPT_FLAGS)
    # Flags for Fortran C and CXX
    set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} ${OPT_FLAGS}")
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPT_FLAGS}")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAGS}")

  else ()

    if (OPT_FFLAGS)
      # User specified optimization flags
      set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} ${OPT_FFLAGS}")
    else ()
      # Defaults
      set(CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE} -O3")
    endif ()

    if (OPT_CFLAGS)
      set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPT_CFLAGS}")
    else ()
      set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
    endif ()

    if (OPT_CXXFLAGS)
      set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_CXXFLAGS}")
    else ()
      if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fopenmp-simd")
      elseif (CMAKE_CXX_COMPILER_ID STREQUAL Intel)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -qopenmp-simd")
      endif ()
    endif ()
  endif ()

  ##############################################################################
  # DEBUG flags
  # If DEBUG_FLAGS is set (to non-empty string), append it to Fortran/C/CXX debug flags.
  # Otherwise, for LANG=F,C,CXX, if DEBUG_<LANG>FLAGS is set (to non-empty string),
  # append it to <LANG> debug flags, otherwise set some defaults.
  ##############################################################################
  if (DEBUG_FLAGS)
    set (CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} ${DEBUG_FLAGS}")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_FLAGS}")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
  else ()
    if (DEBUG_FFLAGS)
      set (CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} ${DEBUG_FFLAGS}")
    else ()
      set (CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -O0")
    endif ()

    if (DEBUG_CFLAGS)
      set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_CFLAGS}")
    else ()
      set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
    endif ()

    if (DEBUG_CXXFLAGS)
      set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_CXXFLAGS}")
    else ()
      if (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -qopenmp-simd -O0")
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fopenmp-simd -O0")
      endif()
    endif ()

  endif ()

  if (EKAT_ENABLE_COVERAGE)
    set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} ${EKAT_COVERAGE_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG} ${EKAT_COVERAGE_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} ${EKAT_COVERAGE_FLAGS}")
  endif()

  option(DEBUG_TRACE "Enables TRACE level debugging checks. Very slow" FALSE)
  if (DEBUG_TRACE)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_TRACE")
  endif ()

  ##############################################################################
  # OpenMP
  ##############################################################################
  if (Kokkos_ENABLE_OPENMP)
    find_package(OpenMP)
    if (TARGET OpenMP::OpenMP_CXX AND TARGET OpenMP::OpenMP_Fortran)
      set (EKAT_OPENMP_TARGETS_FOUND TRUE CACHE BOOL "" INTERNAL)
      message(STATUS "The cmake targets 'OpenMP::OpenMP_CXX' and 'OpenMP::OpenMP_Fortran' will be linked to the ekat cmake target.")
    elseif (OPENMP_FOUND)
      set (EKAT_OPENMP_TARGETS_FOUND FALSE CACHE BOOL "" INTERNAL)
      message(STATUS "The cmake targets 'OpenMP::OpenMP_CXX' and/or 'OpenMP::OpenMP_Fortran' were not found. Setting flags manually.")
      message(STATUS "Found OpenMP Flags")
      message(STATUS "OpenMP_Fortran_FLAGS: ${OpenMP_Fortran_FLAGS}")
      message(STATUS "OpenMP_C_FLAGS: ${OpenMP_C_FLAGS}")
      message(STATUS "OpenMP_CXX_FLAGS: ${OpenMP_CXX_FLAGS}")
      message(STATUS "OpenMP_EXE_LINKER_FLAGS: ${OpenMP_EXE_LINKER_FLAGS}")
      # The fortran openmp flag should be the same as the C Flag
      set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${OpenMP_C_FLAGS}")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    else()
      message(FATAL_ERROR "Unable to find OpenMP")
    endif()
  endif()
endmacro()
