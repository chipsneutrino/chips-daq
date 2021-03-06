#!/bin/bash
##############################################################
# deploy.sh
# Script to stop, distribute and start CHIPS services
##############################################################

source scripts/deploy_config.sh
source scripts/deploy_config_secrets.sh

export BPATH="/tmp/chips-dist.$$/${DIST_DIR_NAME}"
export MACHINES="${DATA_MACHINE} ${MON_MACHINE}"
export DEPLOY_DATE_READABLE=`date -R`
export DEPLOY_VERSION=`git describe --always`

##############################################################

set -e

cp_units() {
	rm -rf "${BPATH}/units"
	mkdir "${BPATH}/units"

	cp ./scripts/systemd/unit.mk "${BPATH}/units/unit.mk"

	UNITS="daqonite daqontrol daqsitter fsm monitoring-tunnel numi-update-watcher spill-tunnel"
	for UNIT in ${UNITS}; do
		mkdir "${BPATH}/units/${UNIT}"
		cp ./scripts/systemd/${UNIT}/Makefile "${BPATH}/units/${UNIT}/Makefile"
		customize_template scripts/systemd/${UNIT}/chips-${UNIT}.service.in units/${UNIT}/chips-${UNIT}.service
	done
}

customize_template() {
	SOURCE_FILE=$1 ; shift
	TARGET_FILE=$1 ; shift
	sed \
		-e "s/%BASE_PATH%/${TGTPATH//\//\\/}\/${DIST_DIR_NAME//\//\\/}/" \
		-e "s/%CONFIG_PATH%/${TGTPATH//\//\\/}\/${CONFIG_DIR_NAME//\//\\/}/" \
		-e "s/%RUN_PATH%/${TGTPATH//\//\\/}\/${RUN_DIR_NAME//\//\\/}/" \
		-e "s/%DIST_CONFIG_PATH%/${TGTPATH//\//\\/}\/${DIST_DIR_NAME//\//\\/}\\/${DIST_CONFIG_DIR_NAME//\//\\/}/" \
		-e "s/%BIN_PATH%/${TGTPATH//\//\\/}\/${DIST_DIR_NAME//\//\\/}\/${DIST_BIN_DIR_NAME//\//\\/}/" \
		-e "s/%DATA_PATH%/${TGTPATH//\//\\/}\/${DATA_DIR_NAME//\//\\/}/" \
		-e "s/%DEPLOY_DATE_READABLE%/${DEPLOY_DATE_READABLE}/" \
		-e "s/%DEPLOY_VERSION%/${DEPLOY_VERSION}/" \
		-e "s/%DATA_MACHINE%/${DATA_MACHINE}/" \
		-e "s/%MON_MACHINE%/${MON_MACHINE}/" \
		-e "s/%ES_PASSWORD%/${ES_PASSWORD}/" \
		-e "s/%RUN_USER%/${RUN_USER}/" \
		./${SOURCE_FILE} \
		>${BPATH}/${TARGET_FILE}
}

cp_artifacts() {
	SRC_PATH="./build"

	rm -rf "${BPATH}/${DIST_BIN_DIR_NAME}" "${BPATH}/lib"

	mkdir "${BPATH}/${DIST_BIN_DIR_NAME}"
	cp ${SRC_PATH}/bin/* "${BPATH}/${DIST_BIN_DIR_NAME}"
	cp ./scripts/run_with_env.sh "${BPATH}/${DIST_BIN_DIR_NAME}"
	cp ./scripts/k5reauth "${BPATH}/${DIST_BIN_DIR_NAME}"
	cp ./scripts/spill_tunnel.sh "${BPATH}/${DIST_BIN_DIR_NAME}"
	cp ./scripts/monitoring_tunnel.sh "${BPATH}/${DIST_BIN_DIR_NAME}"
	cp ./scripts/restart.sh "${BPATH}/${DIST_BIN_DIR_NAME}"
	cp ./scripts/jumbo.sh "${BPATH}/${DIST_BIN_DIR_NAME}"
	customize_template scripts/chips_env_activate.sh.in ${DIST_BIN_DIR_NAME}/chips_env_activate.sh

	mkdir "${BPATH}/lib"
	cp -P ${SRC_PATH}/lib/*.so* "${BPATH}/lib"
	cp -P ${SRC_PATH}/lib/nng/*.so* "${BPATH}/lib"
}

cp_scripts() {
	cp -r ./numi_update_watcher/numi_update_watcher.py "${BPATH}/${DIST_BIN_DIR_NAME}"
}

create_config() {
	mkdir "${BPATH}/config"
	mkdir "${BPATH}/config/global"

	customize_template scripts/dist_config.sh config.sh
	
	customize_template scripts/config/global.cfg.in config/global.cfg
	customize_template scripts/config/global/bus.cfg.in config/global/bus.cfg
	customize_template scripts/config/global/logging.cfg.in config/global/logging.cfg
	customize_template scripts/config/global/elastic.cfg.in config/global/elastic.cfg

	customize_template scripts/config/daqonite.cfg.in config/daqonite.cfg
	customize_template scripts/config/daqulator.cfg.in config/daqulator.cfg
	customize_template scripts/config/daqontrol.cfg.in config/daqontrol.cfg
	customize_template scripts/config/daqsitter.cfg.in config/daqsitter.cfg
	customize_template scripts/config/fsm.cfg.in config/fsm.cfg
	customize_template scripts/config/ops_cmd.cfg.in config/ops_cmd.cfg
	customize_template scripts/config/power.cfg.in config/power.cfg
	customize_template scripts/config/spill_tunnel.cfg.in config/spill_tunnel.cfg
	customize_template scripts/config/monitoring_tunnel.cfg.in config/monitoring_tunnel.cfg
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
			${DEPLOY_USER}@${MACHINE}:${TGTPATH}
		ssh ${DEPLOY_USER}@${MACHINE} "chown -R ${RUN_USER}:${RUN_USER} ${TGTPATH}/${DIST_DIR_NAME}"
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

echo "Will deploy to machines: ${MACHINES}"

mkdir -p "${BPATH}"

cp_units
cp_artifacts
cp_scripts
create_config

./scripts/restart.sh stop
distribute
./scripts/restart.sh start

rm -rf "${BPATH}"

echo "Deployment done."