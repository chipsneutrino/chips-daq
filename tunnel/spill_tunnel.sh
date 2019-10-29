#!/bin/bash

set -e
source ../config.sh

K5REAUTH="${BPATH}/tunnel/k5reauth"
PRINCIPAL="pmanek@FNAL.GOV"
KEYTAB="${BPATH}/tunnel/pmanek.keytab"
AUTOSSH="/usr/bin/autossh"
REMOTE_PORT="17897"
LOCAL_PORT="55812"
REMOTE_HOST="chipsdaq@chipsdaq.dhcp.fnal.gov"

###

${K5REAUTH} \
	-p ${PRINCIPAL} \
	-k ${KEYTAB} \
	-- \
	${AUTOSSH} \
		-M 0 \
		-nNT \
		-R ${REMOTE_PORT}:localhost:${LOCAL_PORT} \
		${REMOTE_HOST}
