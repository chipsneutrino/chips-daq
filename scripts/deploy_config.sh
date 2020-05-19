##############################################################
# deploy_config.sh
# Configuration of the CHIPS deployment used by deploy.sh
##############################################################

# Where to deploy
export TGTPATH="/chips"

# Deployment directory name relative to TGTPATH, must be writable to deploy user and readable to run user
# WARNING: gets wiped upon every deployment!
export DIST_DIR_NAME="dist"

# Data directory name relative to TGTPATH, must be writable to run user (persistent between deployments)
export DATA_DIR_NAME="data"

# Configuration directory name relative to TGTPATH, must be readable to run user (persistent between deployments)
export CONFIG_DIR_NAME="config"

# Directory name relative to TGTPATH, set as working directory of all systemd services, must be readable to run user (persistent between deployments)
export RUN_DIR_NAME="run"

# Configuration directory name relative to DIST_DIR_NAME, must be readable to run user
# WARNING: gets wiped upon every deployment!
export DIST_CONFIG_DIR_NAME="config"

# Directory name relative to DIST_DIR_NAME, where DAQ binaries are stored, must be readable to run user
# WARNING: gets wiped upon every deployment!
export DIST_BIN_DIR_NAME="bin"

# User used to deploy, must be the same on all machines, also must have passwordless login
export DEPLOY_USER="root"

# User used to run, must be the same on all machines, and have appropriate permissions
export RUN_USER="daq"

# Path to rsync (or alternative with similar interface)
export RSYNC="/usr/bin/rsync"

# Which machine is used for data taking
export DATA_MACHINE="localhost" # "chipsshore01"

# Which machine is used for monitoring
export MON_MACHINE="localhost" # "chipsshore04"

# Which services run on the data machine
export DATA_SERVICES="daqonite" #" spill-tunnel"

# Which services run on the monitoring machine
export MON_SERVICES="fsm daqontrol daqsitter" # numi-update-watcher monitoring-tunnel
