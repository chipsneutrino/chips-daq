#!/bin/bash
##############################################################
# deploy.sh
# Script to stop, distribute and start CHIPS services
##############################################################

source scripts/deploy_config.sh

export BPATH="/tmp/chips-dist.$$/${DIST_DIR_NAME}"
export MACHINES="${DATA_MACHINE} ${MON_MACHINE}"

##############################################################

set -e

cp_tunnel() {
	cp -r ./tunnel "${BPATH}"
}

cp_units() {
	cp -r ./units "${BPATH}"
}

customize_template() {
	SOURCE_FILE=$1 ; shift
	TARGET_FILE=$1 ; shift
	sed \
		-e "s/%BASE_PATH%/${TGTPATH//\//\\/}\/${DIST_DIR_NAME//\//\\/}/" \
		./${SOURCE_FILE} \
		>${BPATH}/${TARGET_FILE}
}

cp_artifacts() {
	SRC_PATH="./build"

	rm -rf "${BPATH}/bin" "${BPATH}/lib"

	mkdir "${BPATH}/bin"
	cp ${SRC_PATH}/bin/* "${BPATH}/bin"
	cp ./scripts/run_with_env.sh "${BPATH}/bin"
	cp ./scripts/restart.sh "${BPATH}/bin"
	cp ./scripts/jumbo.sh "${BPATH}/bin"
	customize_template scripts/chips_env_activate.sh.in bin/chips_env_activate.sh

	mkdir "${BPATH}/lib"
	cp -P ${SRC_PATH}/lib/*.so* "${BPATH}/lib"
	cp -P ${SRC_PATH}/lib/nng/*.so* "${BPATH}/lib"
}

cp_scripts() {
	cp -r ./numi_update_watcher/numi_update_watcher.py "${BPATH}/bin"
}

create_config() {
	customize_template scripts/dist_config.sh config.sh
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

./scripts/restart.sh stop
distribute
./scripts/restart.sh start

rm -rf "${BPATH}"
