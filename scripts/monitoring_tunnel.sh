#!/bin/bash
##############################################################
# moniotring_tunnel.sh
# Script to set up and maintain monitoring tunnel to FNAL
##############################################################

source config.sh
set -e

##############################################################

k5reauth \
	-p ${CHIPS_TUNNEL_PRINCIPAL} \
	-k ${CHIPS_TUNNEL_KEYTAB} \
	-- \
	${CHIPS_AUTOSSH} \
		-M 0 \
		-nNT \
		-R ${CHIPS_TUNNEL_MONITORING_REMOTE_PORT}:localhost:${CHIPS_TUNNEL_MONITORING_LOCAL_PORT} \
		${CHIPS_TUNNEL_MONITORING_REMOTE_HOST}
