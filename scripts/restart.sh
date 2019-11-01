#!/bin/bash

if [ $HOSTNAME == 'chipsshore01' ]
then
    echo "Restart DAQ from chipsshore01"
    ssh root@chipsshore04 "systemctl restart chips-fsm; systemctl restart chips-daqontrol; systemctl restart chips-daqsitter;"
    systemctl restart chips-daqonite
    systemctl restart chips-tunnel
elif [ $HOSTNAME == 'chipsshore04' ]
then
    echo "Restart DAQ from chipsshore04"
    systemctl restart chips-fsm
    systemctl restart chips-daqontrol
    systemctl restart chips-daqsitter
    ssh root@chipsshore01 "systemctl restart chips-daqonite; systemctl restart chips-tunnel"
else
    echo "Restart DAQ"
    ssh root@chipsshore04 "systemctl restart chips-fsm; systemctl restart chips-daqontrol; systemctl restart chips-daqsitter;"
    ssh root@chipsshore01 "systemctl restart chips-daqonite; systemctl restart chips-tunnel"
fi

