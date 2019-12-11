##############################################################
# deploy_config.sh
# Configuration of the CHIPS deployment used by deploy.sh
##############################################################

# Where to deploy
export TGTPATH="/chips"

# Deployment directory name in TGTPATH (WARNING: gets wiped!)
export DIST_DIR_NAME="dist"

# User used to deploy, must be the same on all machines, also must have passwordless login
export TGTUSR="root"

# User used to run, must be the same on all machines, and have appropriate permissions
export TGTRUNUSR="daq"

# Path to rsync (or alternative with similar interface)
export RSYNC="/usr/bin/rsync"

# Which machines is used for data taking
export DATA_MACHINE="chipsshore01"

# Which machine is used for monitoring
export MON_MACHINE="chipsshore04"

# Which services run on the data machine
export DATA_SERVICES="daqonite spill-tunnel"

# Which services run on the monitoring machine
export MON_SERVICES="numi-update-watcher fsm daqontrol daqsitter"
