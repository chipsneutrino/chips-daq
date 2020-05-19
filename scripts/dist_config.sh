# Library dependencies
source /opt/gcc-9/activate.sh
source /opt/boost-1.70/activate.sh
source /opt/root-6/activate.sh

####################### BEGIN CHIPS ENVIRONMENT #######################

# Path to the distribution configuration directory (used by all DAQ programs)
export CHIPS_DIST_CONFIG_PATH="%DIST_CONFIG_PATH%"

# Include CHIPS binaries in the executable path
export PATH="%BASE_PATH%/bin:${PATH}"

# Include CHIPS libraries in the dynamic library search path
export LD_LIBRARY_PATH="%BASE_PATH%/lib:${LD_LIBRARY_PATH}"
export LD_RUN_PATH="%BASE_PATH%/lib:${LD_RUN_PATH}"

#######################  END CHIPS ENVIRONMENT  #######################
