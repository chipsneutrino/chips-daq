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
	ssh ${TGTUSR}@${DATA_MACHINE} `awk -v 'RS= ' '{ print "systemctl start chips-" $0 ";" }' <<< "${DATA_SERVICES}"`
    ssh ${TGTUSR}@${MON_MACHINE} `awk -v 'RS= ' '{ print "systemctl start chips-" $0 ";" }' <<< "${MON_SERVICES}"`
}

srv_stop() {
	echo "Stopping DAQ services..."
    ssh ${TGTUSR}@${DATA_MACHINE} `awk -v 'RS= ' '{ print "systemctl stop chips-" $0 ";" }' <<< "${DATA_SERVICES}"`
    ssh ${TGTUSR}@${MON_MACHINE} `awk -v 'RS= ' '{ print "systemctl stop chips-" $0 ";" }' <<< "${MON_SERVICES}"`
}

srv_restart() {
    srv_stop
    srv_start
}

srv_() {
    # Default action
    srv_restart
}

COMMAND=$1 ; shift
srv_${COMMAND}
