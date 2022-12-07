include (EkatUtils)

# Call this at the beginning of a new scope (project/folder),
# so it can overrule parent project/folder default flags for
# debug/release build types for that project/folder.
macro (ResetFlags)
  set(options DEBUG RELEASE COMMON)
  set(args1v)
  set(argsMv)
  cmake_parse_arguments(RF "${options}" "${args1v}" "${argsMv}" ${ARGN})

  if (RF_DEBUG)
    # Reset DEBUG flags
    set (CMAKE_C_FLAGS_DEBUG "")
    set (CMAKE_CXX_FLAGS_DEBUG "")
    set (CMAKE_Fortran_FLAGS_DEBUG "")
  endif()
  if (RF_RELEASE)
    # Reset RELEASE flags
    set (CMAKE_C_FLAGS_RELEASE "")
    set (CMAKE_CXX_FLAGS_RELEASE "")
    set (CMAKE_Fortran_FLAGS_RELEASE "")
  endif()
  if (RF_COMMON)
    # Reset <LANG> flags
    set (CMAKE_C_FLAGS "")
    set (CMAKE_CXX_FLAGS "")
    set (CMAKE_Fortran_FLAGS "")
  endif()
endmacro ()

# Set CMAKE_<LANG>_FLAGS
macro (SetFlags)
  set(options)
  set(args1v)
  set(argsMv FLAGS FFLAGS CFLAGS CXXFLAGS LDFLAGS)
  cmake_parse_arguments(SF "${options}" "${args1v}" "${argsMv}" ${ARGN})

  # TODO: should this be a warning instead?
  if (SF_FLAGS AND (SF_FFLAGS OR SF_CFLAGS OR SF_CXXFLAGS))
    message (FATAL_ERROR "[SetFlags] Input FLAGS arg overrides <LANG>FLAGS")
  endif()

  if (SF_FLAGS)
    # Flags for Fortran C and CXX
    string(APPEND CMAKE_Fortran_FLAGS " ${SF_FLAGS}")
    string(APPEND CMAKE_C_FLAGS " ${SF_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${SF_FLAGS}")
  else ()
    # Fortran
    if (SF_FFLAGS)
      string(APPEND CMAKE_Fortran_FLAGS " ${SF_FFLAGS}")
    endif ()

    # C
    if (SF_CFLAGS)
      string(APPEND CMAKE_C_FLAGS " ${SF_CFLAGS}")
    endif ()

    # CXX
    if (SF_CXXFLAGS)
      string(APPEND CMAKE_CXX_FLAGS " ${SF_CXXFLAGS}")
    endif ()
  endif ()

  # Linker
  if (SF_LDFLAGS)
    string(APPEND CMAKE_EXE_LINKER_FLAGS " ${SF_LDFLAGS}")
  endif()
endmacro()

#############################
# Common basic flags (regardless of build type)
#############################
macro (SetGeneralFlags)
  if (CMAKE_Fortran_COMPILER_ID STREQUAL "Intel")
    SetFlags (FFLAGS "-assume byterecl -ftz" CXXFLAGS -restrict)
  elseif (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
    SetFlags (FFLAGS -ffree-line-length-none)
  endif()
endmacro()

#############################
# Floating point model
#############################
macro (SetFpModelFlags)
  # Set the compiler flag for floating point model,
  # given the Intel name of the model. Supports only GNU and Intel
  # compilers. For GNU, map intel keyword to setting for ffp-contract,
  # with strict->off, everythingelse->fast (the gcc default)
  if (DEFINED ${PROJECT_NAME}_FPMODEL)
    string(TOLOWER "${${PROJECT_NAME}_FPMODEL}" fpmodel_string)
    if (("${fpmodel_string}" STREQUAL "precise") OR
        ("${fpmodel_string}" STREQUAL "strict") OR
        ("${fpmodel_string}" STREQUAL "fast") OR
        ("${fpmodel_string}" STREQUAL "fast=1") OR
        ("${fpmodel_string}" STREQUAL "fast=2"))
      if (CMAKE_Fortran_COMPILER_ID STREQUAL Intel)
        set (FP_MODEL_FLAG "-fp-model ${fpmodel_string}")
      elseif (CMAKE_Fortran_COMPILER_ID STREQUAL GNU)
        if ("${fpmodel_string}" STREQUAL "strict")
          set (FP_MODEL_FLAG -ffp-contract=off)
        else ()
          set (FP_MODEL_FLAG -ffp-contract=fast)
        endif ()
      endif ()
      SetFlags (FLAGS ${FP_MODEL_FLAG})
    elseif (NOT "${fpmodel_string}" STREQUAL "")
      string (CONCAT MSG
              "FP_MODEL_FLAG string '${fpmodel_string}' is not recognized.\n"
              "Valid options: 'precise', 'strict', 'fast', 'fast=1', 'fast=2'.\n"
              "An empty string is also allowed, leaving the choice to the compiler.\n")
      message ("${MSG}")
      message(FATAL_ERROR "Please, use a valid FP model string.")
    endif()
  endif ()
endmacro ()

#############################
# Warnings
#############################
macro (SetWarningFlags)
  # enable all warning but disable Intel vectorization remarks like
  #    "remark: simd loop has only one iteration"
  # since we would get hit with 1000's of those anytime we use Packs with packsize=1.

  if (CMAKE_Fortran_COMPILER_ID STREQUAL "Intel")
    set (SWF_FFLAGS "-warn all -diag-disable=remark -fpscomp logicals")
    set (SWF_CXXFLAGS "-Wall -diag-disable=remark")
  else()
    set (SWF_FFLAGS -Wall)
    set (SWF_CXXFLAGS -Wall)
  endif()

  SetFlags(FFLAGS ${SWF_FFLAGS} CFLAGS -Wall CXXFLAGS ${SWF_CXXFLAGS})
endmacro()

#############################
# Profiling/coverage flags
#############################
macro (SetProfilingFlags)
  set(options)
  set(args1v PROFILER COVERAGE)
  set(argsMv)
  cmake_parse_arguments(SPF "${options}" "${args1v}" "${argsMv}" ${ARGN})

  # NOTE: it is your responsibility to ensure that the profiler CPP macro
  #       works with the current compiler/target, and that --coverage is
  #       recognized by the compiler
  if (SPF_PROFILER)
    string(TOUPPER "${SPF_PROFILER}" PROFILER_UPPER)
    if ("${PROFILER_UPPER}" STREQUAL "VTUNE")
      add_compile_definitions(VTUNE_PROFILE)
    elseif ("${PROFILER_UPPER}" STREQUAL "CUDA")
      add_compile_definitions(CUDA_PROFILE)
    elseif ("${PROFILER_UPPER}" STREQUAL "GPROF")
      add_compile_definitions(GPROF_PROFILE)
      SetFlags(FLAGS -pg LDFLAGS -pg)
    endif ()
  endif()

  if (SPF_COVERAGE)
    SetFlags (FLAGS --coverage LDFLAGS --coverage)
  endif()
endmacro()

#############################
# Optimization flags
#############################
macro (SetReleaseFlags)
  set(options)
  set(args1v)
  set(argsMv FLAGS FFLAGS CFLAGS CXXFLAGS)
  cmake_parse_arguments(SRF "${options}" "${args1v}" "${argsMv}" ${ARGN})

  # TODO: should this be a warning instead?
  if (SRF_FLAGS AND (SRF_FFLAGS OR SRF_CFLAGS OR SRF_CXXFLAGS))
    message (FATAL_ERROR "[SetReleaseFlags] Input FLAGS arg overrides <LANG>FLAGS")
  endif()

  ##############################################################################
  # Optimization flags
  # If SRF_FLAGS is set (to non-empty string), append it to Fortran/C/CXX flags.
  # Otherwise, for LANG=F,C,CXX, if DEBUG_<LANG>FLAGS is set (to non-empty string),
  # append it to <LANG> debug flags, otherwise set some defaults.
  ##############################################################################

  if (SRF_FLAGS)
    # Flags for Fortran C and CXX
    string(APPEND CMAKE_C_FLAGS_RELEASE " ${SRF_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " ${SRF_FLAGS}")
    string(APPEND CMAKE_Fortran_FLAGS_RELEASE " ${SRF_FLAGS}")
  else ()
    # Fortran
    if (SRF_FFLAGS)
      string(APPEND CMAKE_Fortran_FLAGS_RELEASE " ${SRF_FFLAGS}")
    else ()
      string(APPEND CMAKE_Fortran_FLAGS_RELEASE " -O3")
    endif ()

    # C
    if (SRF_CFLAGS)
      string(APPEND CMAKE_C_FLAGS_RELEASE " ${SRF_CFLAGS}")
    else ()
      string(APPEND CMAKE_C_FLAGS_RELEASE " -O3")
    endif ()

    # CXX
    if (SRF_CXXFLAGS)
      string(APPEND CMAKE_CXX_FLAGS_RELEASE " ${SRF_FFLAGS}")
    else ()
      string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")

      # The default flags for CXX include openmp-simd,
      # which is different depending on the compiler
      if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        string (APPEND CMAKE_CXX_FLAGS_RELEASE " -fopenmp-simd")
      elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        string (APPEND CMAKE_CXX_FLAGS_RELEASE " -qopenmp-simd")
      elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        string (APPEND CMAKE_CXX_FLAGS_RELEASE " -fsycl -fsycl-unnamed-lambda -sycl-std=2020 -qopenmp-simd -Wsycl-strict -fsycl-device-code-split=per_kernel")
      endif()
    endif ()
  endif ()
endmacro()

#############################
# DEBUG flags
#############################
macro (SetDebugFlags)
  set(options)
  set(args1v)
  set(argsMv FLAGS FFLAGS CFLAGS CXXFLAGS)
  cmake_parse_arguments(SDF "${options}" "${args1v}" "${argsMv}" ${ARGN})

  # TODO: should this be a warning instead?
  if (SDF_FLAGS AND (SDF_FFLAGS OR SDF_CFLAGS OR SDF_CXXFLAGS))
    message (FATAL_ERROR "[SetReleaseFlags] Input FLAGS arg overrides <LANG>FLAGS")
  endif()

  ##############################################################################
  # DEBUG flags
  # If SDF_FLAGS is set (to non-empty string), append it to Fortran/C/CXX flags.
  # Otherwise, for LANG=F,C,CXX, if SDF_<LANG>FLAGS is set (to non-empty string),
  # append it to <LANG> debug flags, otherwise set some defaults.
  ##############################################################################

  if (SDF_FLAGS)
    # Flags for Fortran C and CXX
    string(APPEND CMAKE_C_FLAGS_DEBUG " ${SDF_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " ${SDF_FLAGS}")
    string(APPEND CMAKE_Fortran_FLAGS_DEBUG " ${SDF_FLAGS}")
  else ()
    # Fortran
    if (SDF_FFLAGS)
      string(APPEND CMAKE_Fortran_FLAGS_DEBUG " ${SDF_FFLAGS}")
    else ()
      string(APPEND CMAKE_Fortran_FLAGS_DEBUG " -g -O0")
    endif ()

    # C
    if (SDF_CFLAGS)
      string(APPEND CMAKE_C_FLAGS_DEBUG " ${SDF_CFLAGS}")
    else ()
      string(APPEND CMAKE_C_FLAGS_DEBUG " -g -O0")
    endif ()

    # CXX
    if (SDF_CXXFLAGS)
      string(APPEND CMAKE_CXX_FLAGS_DEBUG " ${SDF_FFLAGS}")
    else ()
      string(APPEND CMAKE_CXX_FLAGS_DEBUG " -g -O0")

      # The default flags for CXX include openmp-simd,
      # which is different depending on the compiler
      if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        string (APPEND CMAKE_CXX_FLAGS_DEBUG " -fopenmp-simd")
      elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        string (APPEND CMAKE_CXX_FLAGS_DEBUG " -qopenmp-simd")
      elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        string (APPEND CMAKE_CXX_FLAGS_DEBUG " -qopenmp-simd -fsycl -fsycl-unnamed-lambda -sycl-std=2020 -Wsycl-strict -fno-sycl-dead-args-optimization")
      endif()
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

    if (NOT OMP_C AND NOT OMP_CXX AND NOT_OMP_Fortran)
      message (FATAL_ERROR "[SetOmpFlags] You need to specify at least one language")
    endif()

    set (langReq)
    set (omp_targets)
    foreach (lang C CXX Fortran)
      if (OMP_${lang})
        list (APPEND langReq ${lang})
      endif()
    endforeach()

    find_package(OpenMP REQUIRED COMPONENTS ${langReq})

    # Check if OpenMP_<lang> are available as imported targets
    set (found_omp_targets TRUE)
    foreach (lang IN LISTS langReq)
      # The following should not be needed, but just in case we are
      # picking up a FindOpenMP.cmake module that doesn't use _FIND_REQUIRED
      if (NOT OpenMP_${lang}_FOUND)
        message (FATAL_ERROR "[SetOmpFlags] Looks like the compiler is lacking ${lang} OpenMP support.")
      endif()
      if (NOT TARGET OpenMP::OpenMP_${lang})
        set (all_omp_targets FALSE)
      endif()
    endforeach()

    # If OpenMP_<lang> targets are available, simply link, otherwise manually set flags
    if (found_omp_targets)
      foreach (lang IN LISTS langReq)
        target_link_libraries(${targetName} PUBLIC OpenMP::OpenMP_${lang})
      endforeach()
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

    # Still check if CUDA_FOUND is true, since we don't know if the particular
    # FindCUDA.cmake module being used is checking _FIND_REQUIRED
    if (NOT CUDA_FOUND)
      message (FATAL_ERROR "Error! Unable to find CUDA.")
    endif()

    set(options CUDA_LANG)
    set(args1v)
    set(argsMv FLAGS)
    cmake_parse_arguments(SCF "${options}" "${args1v}" "${argsMv}" ${ARGN})

    if (SCF_FLAGS)
      set (FLAGS ${SCF_FLAGS})
    else ()
      # We need host-device lambdas
      set (FLAGS --expt-extended-lambda)

      IsDebugBuild (SCF_DEBUG)
      if (SCF_DEBUG)
        # Turn off fused multiply add for debug so we can stay BFB with host
        list (APPEND FLAGS --fmad=false)
      endif()
    endif()

    # Set the flags on the target
    if (SCF_CUDA_LANG)
      # User is setting the src files language to CUDA
      target_compile_options (${targetName} PUBLIC
        "$<$<COMPILE_LANGUAGE:CUDA>:${FLAGS}>")
    else()
      # We assume the user is setting the src files lang to CXX
      target_compile_options (${targetName} PUBLIC
        "$<$<COMPILE_LANGUAGE:CXX>:${FLAGS}>")
    endif()
  endif()
endmacro()

##############################################################################
# Convenience macro, to process all common flags setting all defaults
##############################################################################
macro (SetCommonFlags)
  SetGeneralFlags()
  SetFpModelFlags()
  SetWarningFlags()
  SetDebugFlags()
  SetReleaseFlags()
endmacro()
