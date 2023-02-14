#! /bin/bash -xe

export JENKINS_SCRIPT_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
DATE_STAMP=$(date "+%Y-%m-%d_%H%M%S")

if [ -z "$WORKSPACE" ]; then
    echo "Must run from Jenkins job"
fi

cd $WORKSPACE/${BUILD_ID}/

WORK_DIR=$(pwd)

# setup env, use SCREAM env
SCREAM_SCRIPTS=${WORK_DIR}/scream/components/eamxx/scripts
source ${SCREAM_SCRIPTS}/jenkins/${NODE_NAME}_setup
source ${SCREAM_SCRIPTS}/source_to_load_scream_env.sh

BATCHP=$(${SCREAM_SCRIPTS}/query-scream $SCREAM_MACHINE batch)

set -o pipefail
$BATCHP $JENKINS_SCRIPT_DIR/jenkins_impl.sh 2>&1 | tee JENKINS_$DATE_STAMP
