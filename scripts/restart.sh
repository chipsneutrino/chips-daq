#!/bin/bash
##############################################################
# restart.sh
# Script to stop and start CHIPS services
##############################################################

source scripts/deploy_config.sh

##############################################################

set -e

srv_start() {
	echo "Starting DAQ services..."
	ssh ${DEPLOY_USER}@${DATA_MACHINE} `awk -v 'RS= ' '{ print "systemctl start chips-" $0 ";" }' <<< "${DATA_SERVICES}"`
    ssh ${DEPLOY_USER}@${MON_MACHINE} `awk -v 'RS= ' '{ print "systemctl start chips-" $0 ";" }' <<< "${MON_SERVICES}"`
}

srv_stop() {
	echo "Stopping DAQ services..."
    ssh ${DEPLOY_USER}@${DATA_MACHINE} `awk -v 'RS= ' '{ print "systemctl stop chips-" $0 ";" }' <<< "${DATA_SERVICES}"`
    ssh ${DEPLOY_USER}@${MON_MACHINE} `awk -v 'RS= ' '{ print "systemctl stop chips-" $0 ";" }' <<< "${MON_SERVICES}"`
}

srv_restart() {
    srv_stop
    srv_start
}

if [ $# == 0 ]; then
    # Default
    COMMAND="restart"
else
    COMMAND=$1 ; shift
fi

echo "Requested command: ${COMMAND}"
srv_${COMMAND}
