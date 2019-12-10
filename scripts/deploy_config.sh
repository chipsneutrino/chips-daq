# Where to deploy
export TGTPATH="/opt"

# User used to deploy
export TGTUSR="root"

# Path to rsync (or alternative with similar interface)
export RSYNC="/usr/bin/rsync"

# Which machines is used for data taking
export DATA_MACHINE="chipsshore01"

# Which machine is used for monitoring
export MON_MACHINE="chipsshore04"

# Which services run on the data machine
export DATA_SERVICES="daqonite tunnel"

# Which services run on the monitoring machine
export MON_SERVICES="numi-update-watcher fsm daqontrol daqsitter"
