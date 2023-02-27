#! /bin/bash -xe

EKAT_JENKINS_SCRIPT_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
DATE_STAMP=$(date "+%Y-%m-%d_%H%M%S")

if [ -z "$WORKSPACE" ]; then
    echo "Must run from Jenkins job"
    exit 1
fi

cd $WORKSPACE/${BUILD_ID}

export WORK_DIR=$(pwd)

# setup env, use SCREAM env
export SCREAM_SCRIPTS=${WORK_DIR}/scream/components/eamxx/scripts
export JENKINS_SCRIPT_DIR=${SCREAM_SCRIPTS}/jenkins # some scream env setups depend on this
source ${SCREAM_SCRIPTS}/jenkins/${NODE_NAME}_setup
source ${SCREAM_SCRIPTS}/source_to_load_scream_env.sh

if [ -z "$SCREAM_MACHINE" ]; then
    echo "SCREAM_MACHINE must be set by ${SCREAM_SCRIPTS}/jenkins/${NODE_NAME}_setup in order for jenkins infrastructure to work"
    exit 1
fi

export SCREAM_MACHINE

BATCHP=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE batch)

set -o pipefail
$BATCHP $EKAT_JENKINS_SCRIPT_DIR/jenkins_impl.sh 2>&1 | tee JENKINS_$DATE_STAMP
