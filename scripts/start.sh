#!/bin/bash

if [ $# -eq 0 ]
then
    echo "Need config file!"
else
    echo "Starting DAQ with config: "
    echo $1

    echo "Starting FSM ..."
    apps/fsm.sh > apps/log.txt &

    echo "Starting DAQontrol ..."
    apps/daqontrol.sh $1 > apps/log.txt &

    echo "Starting DAQsitter ..."
    apps/daqsitter.sh > apps/log.txt &

    echo "Starting DAQonite ..."
    apps/daqonite.sh > apps/log.txt &
fi
