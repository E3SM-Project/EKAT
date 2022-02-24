include (EkatUtils)

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

#############################
# Floating point model
#############################
macro (SetFpModelFlags targetName)
  if (DEFINED ${PROJECT_NAME}_FPMODEL)
    EKAT_set_fpmodel_flags("${${PROJECT_NAME}_FPMODEL}" FP_MODEL_FLAG)
    string(TOLOWER "${${PROJECT_NAME}_FPMODEL}" fpmodel_string_lower)
    if (fpmodel_string_lower STREQUAL "strict")
      set (${PROJECT_NAME}_STRICT_FP TRUE PARENT_SCOPE BOOL)
    endif()
    target_compile_options (targetName PRIVATE ${FP_MODEL_FLAG})
  endif ()
endmacro ()

#############################
# Warnings
#############################
macro (SetWarningFlags targetName)
  # TODO: should we remove this altogether?
  set(options PRIVATE)
  set(args1v)
  set(argsMv)
  cmake_parse_arguments(SWF "${options}" "${args1v}" "${argsMv}" ${ARGN})

  if (SWF_PRIVATE)
    set (scope PRIVATE)
  else()
    set (scope PUBLIC)
  endif()
  
  # enable all warning but disable Intel vectorization remarks like
  #    "remark: simd loop has only one iteration"
  # since we would get hit with 1000's of those anytime we use Packs with packsize=1.

  target_compile_options (${targetName} ${scope} $<$<COMPILE_LANGUAGE:C>:-Wall>)
  target_compile_options (${targetName} ${scope} $<$<COMPILE_LANGUAGE:CXX>:-Wall>)

  # Some flags need a switch based on compiler id
  target_compile_options (${targetName} ${scope}
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<Fortran_COMPILER_ID:Intel>>:-diag-disable=remark>)
  target_compile_options (${targetName} ${scope}
    $<$<AND:$<COMPILE_LANGUAGE:Fortran>,$<Fortran_COMPILER_ID:Intel>>:-warn all -diag-disable=remark -fpscomp logicals>)

  target_compile_options (${targetName} ${scope}
    $<$<AND:$<COMPILE_LANGUAGE:Fortran>,$<Fortran_COMPILER_ID:GNU>>:-Wall>)
endmacro()

#############################
# Common basic flags (regardless of build type)
#############################
macro (SetBasicFlags targetName)
  # Fortran Flags
  target_compile_options (${targetName} PUBLIC
    $<$<AND:$<COMPILE_LANGUAGE:Fortran>,$<Fortran_COMPILER_ID:GNU>>:-ffree-line-length-none>)
  target_compile_options (${targetName} PUBLIC
    $<$<AND:$<COMPILE_LANGUAGE:Fortran>,$<Fortran_COMPILER_ID:Intel>>:-assume byterecl -ftz>)

  # C++ Flags
  target_compile_options (${targetName} PUBLIC
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:Intel>>:-restrict>)
endmacro()

#############################
# Profiling/coverage flags
#############################
macro (SetProfilingFlags targetName)
  set(options COVERAGE)
  set(args1v PROFILER)
  set(argsMv)
  cmake_parse_arguments(SPF "${options}" "${args1v}" "${argsMv}" ${ARGN})

  # NOTE: it is your responsibility to ensure that the profiler CPP macro
  #       works with the current compiler/target, and that --coverage is
  #       recognized by the compiler
  if (SPF_PROFILER)
    string(TOUPPER "${SPF_PROFILER}" PROFILER_UPPER)
    if ("${PROFILER_UPPER}" STREQUAL "VTUNE")
      target_compile_definitions(${targetName} PUBLIC VTUNE_PROFILE)
    elseif ("${PROFILER_UPPER}" STREQUAL "CUDA")
      target_compile_definitions(${targetName} PUBLIC CUDA_PROFILE)
    elseif ("${PROFILER_UPPER}" STREQUAL "GPROF")
      target_compile_definitions(${targetName} PUBLIC GPROF_PROFILE)
      target_compile_options(${targetName} PUBLIC -pg)
      target_link_options (${targetName} PUBLIC -pg)
    endif ()
  endif()

  if (SPF_COVERAGE)
    target_compile_options (${targetName} PUBLIC --coverage)
    target_link_options (${targetName} PUBLIC --coverage)
  endif()
endmacro()

#############################
# Optimization flags
#############################
macro (SetOptFlags targetName)
  set(options)
  set(args1v)
  set(argsMv FLAGS FFLAGS CFLAGS CXXFLAGS)
  cmake_parse_arguments(OPT "${options}" "${args1v}" "${argsMv}" ${ARGN})

  # TODO: should this be a warning instead?
  if (OPT_FLAGS AND (OPT_FFLAGS OR OPT_CFLAGS OR OPT_CXXFLAGS))
    message (FATAL_ERROR "[SetOptFlags] Input FLAGS arg overrides <LANG>FLAGS")
  endif()

  ##############################################################################
  # Optimization flags
  # If OPT_FLAGS is set (to non-empty string), append it to Fortran/C/CXX flags.
  # Otherwise, for LANG=F,C,CXX, if DEBUG_<LANG>FLAGS is set (to non-empty string),
  # append it to <LANG> debug flags, otherwise set some defaults.
  ##############################################################################

  if (OPT_FLAGS)
    # Flags for Fortran C and CXX
    target_compile_options(${targetName} PUBLIC ${OPT_FLAGS})
  else ()
    # Fortran
    if (OPT_FFLAGS)
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:${OPT_FFLAGS}>)
    else ()
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-O3>)
    endif ()

    # C
    if (OPT_CFLAGS)
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:C>:${OPT_CFLAGS}>)
    else ()
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:C>:-O3>)
    endif ()

    # CXX
    if (OPT_CXXFLAGS)
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${OPT_CXXFLAGS}>)
    else ()
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-O3>)
      target_compile_options(${targetName} PUBLIC
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU>>:-fopenmp-simd>)
      target_compile_options(${targetName} PUBLIC
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:Intel>>:-qopenmp-simd>)
    endif ()
  endif ()
endmacro()

#############################
# DEBUG flags
#############################
macro (SetDebugFlags targetName)
  set(options)
  set(args1v)
  set(argsMv FLAGS FFLAGS CFLAGS CXXFLAGS)
  cmake_parse_arguments(DEBUG "${options}" "${args1v}" "${argsMv}" ${ARGN})

  # TODO: should this be a warning instead?
  if (DEBUG_FLAGS AND (DEBUG_FFLAGS OR DEBUG_CFLAGS OR DEBUG_CXXFLAGS))
    message (FATAL_ERROR "[SetDebugFlags] Input FLAGS arg overrides <LANG>FLAGS")
  endif()

  ##############################################################################
  # DEBUG flags
  # If DEBUG_FLAGS is set (to non-empty string), append it to Fortran/C/CXX debug flags.
  # Otherwise, for LANG=F,C,CXX, if DEBUG_<LANG>FLAGS is set (to non-empty string),
  # append it to <LANG> debug flags, otherwise set some defaults.
  ##############################################################################
  if (DEBUG_FLAGS)
    # Flags for Fortran C and CXX
    target_compile_options(${targetName} PUBLIC ${DEBUG_FLAGS})
  else ()
    # Fortran
    if (DEBUG_FFLAGS)
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:${DEBUG_FFLAGS}>)
    else ()
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-O0>)
    endif ()

    # C
    if (DEBUG_CFLAGS)
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:C>:${DEBUG_CFLAGS}>)
    else ()
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:C>:-O0>)
    endif ()

    # CXX
    if (DEBUG_CXXFLAGS)
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${DEBUG_CXXFLAGS}>)
    else ()
      target_compile_options(${targetName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-O0>)
      target_compile_options(${targetName} PUBLIC
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU>>:-fopenmp-simd>)
      target_compile_options(${targetName} PUBLIC
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:Intel>>:-qopenmp-simd>)
    endif ()
  endif ()
endmacro()

##############################################################################
# OpenMP
##############################################################################
macro (SetOmpFlags targetName)
  if (Kokkos_ENABLE_OPENMP)
    set(options C CXX Fortran)
    set(args1v)
    set(argsMv FLAGS FFLAGS CFLAGS CXXFLAGS)
    cmake_parse_arguments(OMP "${options}" "${args1v}" "${argsMv}" ${ARGN})

    set (langReq)
    set (all_omp_targets TRUE)
    set (omp_targets)
    foreach (lang "C;CXX;Fortran")
      if (OMP_${lang})
        list (APPEND langReq ${lang})
        list (APPEND omp_targets OpenMP::OpenMP_${lang})
        if (NOT TARGET OpenMP::OpenMP_${lang})
          set (all_omp_targets FALSE)
        endif()
      endif()
    endforeach()

    find_package(OpenMP REQUIRED COMPONENTS ${langReq})
    if (all_omp_targets)
      target_link_libraries(${targetName} PUBLIC OpenMP::OpenMP_${lang})
      message(STATUS "The cmake targets ${omp_targets} will be linked to ${targetName}.")
    elseif (OpenMP_${lang}_FOUND)
      message(STATUS "One of the cmake targets ${omp_targets} was not found. Setting flags manually.")

      foreach (lang IN LISTS langReq)
        message(STATUS "OpenMP_${lang}_FLAGS: ${OpenMP_${lang}_FLAGS}")
        target_compile_options (${targetName} PUBLIC ${OpenMP_${lang}_FLAGS})
      endforeach()
      message(STATUS "OpenMP_EXE_LINKER_FLAGS: ${OpenMP_EXE_LINKER_FLAGS}")
      target_link_options (${targetName} PUBLIC ${OpenMP_EXE_LINKER_FLAGS})
    else()
      message(FATAL_ERROR "Unable to find OpenMP for language ${lang}")
    endif()
  endif()
endmacro()

##############################################################################
# CUDA
##############################################################################
macro (SetCudaFlags targetName)
  if (Kokkos_ENABLE_CUDA)
    # We must find CUDA
    find_package(CUDA REQUIRED)

    # Still check if CUDA-NOTFOUDN is set, since find_package in Module mode
    # need not to error out if package is not found
    if (CUDA-NOTFOUND)
      message (FATAL_ERROR "Error! Unable to find CUDA.")
    endif()

    IsDebugBuild (SCF_DEBUG)
    if (SCF_DEBUG)
      # Turn off fused multiply add for debug so we can stay BFB with host
      target_compile_options ($[targetName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:"--fmad=false">)
    endif()

    # We need host-device lambdas
    target_compile_options ($[targetName} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:"--expt-extended-lambda">)
  endif()
endmacro()
