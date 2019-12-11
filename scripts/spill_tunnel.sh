#!/bin/bash
##############################################################
# spill_tunnel.sh
# Script to set up and maintain spill tunnel to FNAL
##############################################################

source config.sh
set -e

###

k5reauth \
	-p ${CHIPS_TUNNEL_PRINCIPAL} \
	-k ${CHIPS_TUNNEL_KEYTAB} \
	-- \
	${CHIPS_AUTOSSH} \
		-M 0 \
		-nNT \
		-R ${CHIPS_TUNNEL_SPILL_REMOTE_PORT}:localhost:${CHIPS_TUNNEL_SPILL_LOCAL_PORT} \
		${CHIPS_TUNNEL_SPILL_REMOTE_HOST}
