#!/bin/bash

set -e

source scripts/deploy_config.sh

export BPATH="/tmp/chips-dist.$$/${DIST_DIR_NAME}"
export MACHINES="${DATA_MACHINE} ${MON_MACHINE}"

cp_tunnel() {
	cp -r ./tunnel "${BPATH}"
}

cp_units() {
	cp -r ./units "${BPATH}"
}

cp_artifacts() {
	SRC_PATH="./build"

	rm -rf "${BPATH}/bin" "${BPATH}/lib"

	mkdir "${BPATH}/bin"
	cp ${SRC_PATH}/bin/* "${BPATH}/bin"
	cp ./scripts/run_with_env.sh "${BPATH}/bin"
	cp ./scripts/restart.sh "${BPATH}/bin"
	cp ./scripts/jumbo.sh "${BPATH}/bin"

	mkdir "${BPATH}/lib"
	cp -P ${SRC_PATH}/lib/*.so* "${BPATH}/lib"
	cp -P ${SRC_PATH}/lib/nng/*.so* "${BPATH}/lib"
}

cp_scripts() {
	cp -r ./numi_update_watcher/numi_update_watcher.py "${BPATH}/bin"
}

create_config() {
	sed \
		-e "s/%BASE_PATH%/${TGTPATH//\//\\/}\/${DIST_DIR_NAME//\//\\/}/" \
		./scripts/dist_config.sh \
		>${BPATH}/config.sh
}

srv_stop() {
	echo "Stopping DAQ services..."
    ssh ${TGTUSR}@${DATA_MACHINE} `awk -v 'RS= ' '{ print "systemctl stop chips-" $0 ";" }' <<< "${DATA_SERVICES}"`
    ssh ${TGTUSR}@${MON_MACHINE} `awk -v 'RS= ' '{ print "systemctl stop chips-" $0 ";" }' <<< "${MON_SERVICES}"`
}

distribute() {
	for MACHINE in $MACHINES ; do
		echo "Deploying to ${MACHINE}..."
		${RSYNC} \
			--archive \
			--verbose \
			--compress \
			--delete \
			${BPATH} \
			${TGTUSR}@${MACHINE}:${TGTPATH}
		ssh ${TGTUSR}@${MACHINE} "chown -R ${TGTRUNUSR}:${TGTRUNUSR} ${TGTPATH}/${DIST_DIR_NAME}"
	done
}

srv_start() {
	echo "Starting DAQ services..."
	ssh ${TGTUSR}@${DATA_MACHINE} `awk -v 'RS= ' '{ print "systemctl start chips-" $0 ";" }' <<< "${DATA_SERVICES}"`
    ssh ${TGTUSR}@${MON_MACHINE} `awk -v 'RS= ' '{ print "systemctl start chips-" $0 ";" }' <<< "${MON_SERVICES}"`
}

if [ ! -d .git ]; then
	echo "The working directory should be the entire repo."
	exit 1
fi

if [ ! -d build ]; then
	echo "The directory with build artifacts is expected to be called 'build'"
	exit 2
fi

mkdir -p "${BPATH}"

cp_tunnel
cp_units
cp_artifacts
cp_scripts
create_config

srv_stop
distribute
srv_start

rm -rf "${BPATH}"
