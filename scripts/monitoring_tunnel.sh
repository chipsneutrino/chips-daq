#!/bin/bash
##############################################################
# moniotring_tunnel.sh
# Script to set up and maintain monitoring tunnel to FNAL
##############################################################

source config.sh
source ${CHIPS_DIST_CONFIG_PATH}/monitoring_tunnel.cfg
set -e

##############################################################

k5reauth \
	-p ${fnal_principal} \
	-k ${fnal_keytab} \
	-- \
	${autossh} \
		-M 0 \
		-nNT \
		-R ${remote_port}:localhost:${local_port} \
		${remote_host}
