#! /bin/bash -x

if [ -z "$WORKSPACE" ]; then
    echo "Must run from Jenkins job"
fi

cd $WORKSPACE/${BUILD_ID}/

WORK_DIR=$(pwd)

rm -rf ekat-build ekat-install

# setup env, use SCREAM env
SCREAM_SCRIPTS=${WORK_DIR}/scream/components/scream/scripts
source ${SCREAM_SCRIPTS}/jenkins/${NODE_NAME}_setup
source ${SCREAM_SCRIPTS}/source_to_load_scream_env.sh

# Merge origin master, to make sure we're up to date. If merge fails, exit.
cd ${WORK_DIR}/ekat-src;
git log -1
git merge origin/master && cd ${WORK_DIR}
if [ $? -ne 0 ]; then
    echo "Error trying to merge origin/master"
    exit 1;
fi

# Query scream for machine info
MPICXX=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE cxx_compiler)
MPICC=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE c_compiler)
MPIF90=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE f90_compiler)
BATCHP=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE batch)
COMP_J=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE comp_j)
TEST_J=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE test_j)
ISCUDA=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE cuda)

# We create separate builds for single precision (SP), double precision (DP),
# DP with floating point exceptions enabled (FPE), and, on CUDA, DP with
# Cuda Unified Virtual Memory (UVM) as memory space.

FAILED_SP=""
FAILED_DP=""
FAILED_FPE=""
FAILED_UVM=""
RET_SP=0
RET_DP=0
RET_FPE=0
RET_UVM=0

export CTEST_PARALLEL_LEVEL=${TEST_J}
EKAT_THREAD_SETTINGS=""
if [[ "$ISCUDA" == "False" ]]; then
    EKAT_THREAD_SETTINGS="-DEKAT_TEST_THREAD_INC=2 -DEKAT_TEST_MAX_THREADS=14"
fi

# Build and test double precision
mkdir -p ekat-build/ekat-sp && cd ekat-build/ekat-sp && rm -rf *

cmake -C ${WORK_DIR}/ekat-src/cmake/machine-files/${NODE_NAME}.cmake \
    -DCMAKE_INSTALL_PREFIX=${WORK_DIR}/ekat-install/ekat-sp    \
    -DCMAKE_BUILD_TYPE=DEBUG                                   \
    -DCMAKE_C_COMPILER=${MPICC}                                \
    -DCMAKE_CXX_COMPILER=${MPICXX}                             \
    -DCMAKE_Fortran_COMPILER=${MPIF90}                         \
    -DEKAT_DISABLE_TPL_WARNINGS=ON                             \
    -DEKAT_ENABLE_FPE_DEFAULT_MASK=OFF                         \
    -DEKAT_ENABLE_TESTS=ON                                     \
    -DEKAT_TEST_DOUBLE_PRECISION=OFF                           \
    -DEKAT_TEST_SINGLE_PRECISION=ON                            \
    ${EKAT_THREAD_SETTINGS}                                    \
    ${WORK_DIR}/ekat-src

if [ $? -ne 0 ]; then
    echo "Something went wrong while configuring the SP case."
    RET_SP=1
else
    ${BATCHP} make -j ${COMP_J}
    if [ $? -ne 0 ]; then
        echo "Something went wrong while building the SP case."
        RET_SP=1
    else
        ${BATCHP} ctest --output-on-failure
        if [ $? -ne 0 ]; then
            echo "Something went wrong while testing the SP case."
            RET_SP=1
            FAILED_SP=$(cat Testing/Temporary/LastTestsFailed.log)
        else
            make install
            if [ $? -ne 0 ]; then
                echo "Something went wrong while installing the SP case."
                RET_SP=1
            fi
        fi
    fi
fi
cd ${WORK_DIR}

# Build and test single precision
mkdir -p ekat-build/ekat-dp && cd ekat-build/ekat-dp && rm -rf *

cmake -C ${WORK_DIR}/ekat-src/cmake/machine-files/${NODE_NAME}.cmake \
    -DCMAKE_INSTALL_PREFIX=${WORK_DIR}/ekat-install/ekat-dp    \
    -DCMAKE_BUILD_TYPE=DEBUG                                   \
    -DCMAKE_C_COMPILER=${MPICC}                                \
    -DCMAKE_CXX_COMPILER=${MPICXX}                             \
    -DCMAKE_Fortran_COMPILER=${MPIF90}                         \
    -DEKAT_DISABLE_TPL_WARNINGS=ON                             \
    -DEKAT_ENABLE_TESTS=ON                                     \
    -DEKAT_ENABLE_FPE_DEFAULT_MASK=OFF                         \
    -DEKAT_TEST_DOUBLE_PRECISION=ON                            \
    -DEKAT_TEST_SINGLE_PRECISION=OFF                           \
    ${EKAT_THREAD_SETTINGS}                                    \
    ${WORK_DIR}/ekat-src

if [ $? -ne 0 ]; then
    echo "Something went wrong while configuring the DP case."
    RET_DP=1
else
    ${BATCHP} make -j ${COMP_J}
    if [ $? -ne 0 ]; then
        echo "Something went wrong while building the DP case."
        RET_DP=1
    else
        ${BATCHP} ctest --output-on-failure
        if [ $? -ne 0 ]; then
            echo "Something went wrong while testing the DP case."
            RET_DP=1
            FAILED_DP=$(cat Testing/Temporary/LastTestsFailed.log)
        else
            make install
            if [ $? -ne 0 ]; then
                echo "Something went wrong while installing the DP case."
                RET_DP=1
            fi
        fi
    fi
fi
cd ${WORK_DIR}

if [[ "$ISCUDA" == "False" ]]; then
  # Build and test double precision with FPE on, and packsize=1
  mkdir -p ekat-build/ekat-fpe && cd ekat-build/ekat-fpe && rm -rf *

  cmake -C ${WORK_DIR}/ekat-src/cmake/machine-files/${NODE_NAME}.cmake \
      -DCMAKE_INSTALL_PREFIX=${WORK_DIR}/ekat-install/ekat-fpe   \
      -DCMAKE_BUILD_TYPE=DEBUG                                   \
      -DCMAKE_C_COMPILER=${MPICC}                                \
      -DCMAKE_CXX_COMPILER=${MPICXX}                             \
      -DCMAKE_Fortran_COMPILER=${MPIF90}                         \
      -DEKAT_DISABLE_TPL_WARNINGS=ON                             \
      -DEKAT_ENABLE_TESTS=ON                                     \
      -DEKAT_ENABLE_FPE_DEFAULT_MASK=ON                          \
      -DEKAT_TEST_PACK_SIZE=1                                    \
      -DEKAT_TEST_DOUBLE_PRECISION=ON                            \
      -DEKAT_TEST_SINGLE_PRECISION=OFF                           \
      ${EKAT_THREAD_SETTINGS}                                    \
      ${WORK_DIR}/ekat-src

  if [ $? -ne 0 ]; then
      echo "Something went wrong while configuring the FPE case."
      RET_FPE=1
  else
      ${BATCHP} make -j ${COMP_J}
      if [ $? -ne 0 ]; then
          echo "Something went wrong while building the FPE case."
          RET_FPE=1
      else
          ${BATCHP} ctest --output-on-failure
          if [ $? -ne 0 ]; then
              echo "Something went wrong while testing the FPE case."
              RET_FPE=1
              FAILED_FPE=$(cat Testing/Temporary/LastTestsFailed.log)
          else
              make install
              if [ $? -ne 0 ]; then
                  echo "Something went wrong while installing the FPE case."
                  RET_FPE=1
              fi
          fi
      fi
  fi
  cd ${WORK_DIR}
fi

if [[ "$ISCUDA" == "True" ]]; then
  # Build and test Cuda UVM
  mkdir -p ekat-build/ekat-uvm && cd ekat-build/ekat-uvm && rm -rf *

  cmake -C ${WORK_DIR}/ekat-src/cmake/machine-files/${NODE_NAME}.cmake \
      -DCMAKE_INSTALL_PREFIX=${WORK_DIR}/ekat-install/ekat-uvm   \
      -DCMAKE_BUILD_TYPE=DEBUG                                   \
      -DCMAKE_C_COMPILER=${MPICC}                                \
      -DCMAKE_CXX_COMPILER=${MPICXX}                             \
      -DCMAKE_Fortran_COMPILER=${MPIF90}                         \
      -DEKAT_DISABLE_TPL_WARNINGS=ON                             \
      -DEKAT_ENABLE_TESTS=ON                                     \
      -DEKAT_TEST_DOUBLE_PRECISION=ON                            \
      -DEKAT_TEST_SINGLE_PRECISION=OFF                           \
      -DKokkos_ENABLE_CUDA_UVM=ON                                \
      ${EKAT_THREAD_SETTINGS}                                    \
      ${WORK_DIR}/ekat-src

  if [ $? -ne 0 ]; then
      echo "Something went wrong while configuring the UVM case."
      RET_UVM=1
  else
      ${BATCHP} make -j ${COMP_J}
      if [ $? -ne 0 ]; then
          echo "Something went wrong while building the UVM case."
          RET_UVM=1
      else
          ${BATCHP} ctest --output-on-failure
          if [ $? -ne 0 ]; then
              echo "Something went wrong while testing the UVM case."
              RET_UVM=1
              FAILED_UVM=$(cat Testing/Temporary/LastTestsFailed.log)
          else
              make install
              if [ $? -ne 0 ]; then
                  echo "Something went wrong while installing the UVM case."
                  RET_UVM=1
              fi
          fi
      fi
  fi
  cd ${WORK_DIR}
fi

# Print list of failed tests
if [[ $RET_SP -ne 0 && "$FAILED_SP" != "" ]]; then
    echo "List of failed SP tests:"
    echo "$FAILED_SP"
fi

if [[ $RET_DP -ne 0 && "$FAILED_DP" != "" ]]; then
    echo "List of failed DP tests:"
    echo "$FAILED_DP"
fi

if [[ $RET_FPE -ne 0 && "$FAILED_FPE" != "" ]]; then
    echo "List of failed FPE tests:"
    echo "$FAILED_FPE"
fi

if [[ $RET_UVM -ne 0 && "$FAILED_UVM" != "" ]]; then
    echo "List of failed DP tests:"
    echo "$FAILED_UVM"
fi

# Check if all builds succeded, and establish success/fail
if [[ $RET_SP -ne 0 || $RET_DP -ne 0 || $RET_FPE -ne 0 || $RET_UVM -ne 0 ]]; then
    exit 1;
fi

# All good, return 0
exit 0;
