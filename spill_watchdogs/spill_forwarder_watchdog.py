#!/usr/bin/env python3

import subprocess
import time

# TODO: configure from environment
FORWARDER_PROC_NAME = ['NssSpillForwarder', '-c NovaSpillServer/config/SpillForwarderConfig-CHIPS.xml', '-D 0', '-M 0']
FORWARDER_HOLDOFF_SEC = 1

def run_forwarder():
    running_forwarder = True

    while running_forwarder:
        forwarder = subprocess.run(FORWARDER_PROC_NAME)
        # TODO: react on `forwarder.returncode`

        time.sleep(FORWARDER_HOLDOFF_SEC)

def run_loopback_receiver():
    # TODO: run XML-RPC server
    # TODO: keep track of last signal received grouped by type
    pass

def run_fsm_reporter():
    # TODO: run on timer
    # TODO: check that sender PID on PPC is valid
    # TODO: check that forwarder is running
    # TODO: check that all loopback times are fresh
    # TODO: send heartbeat through NNG to FSM
    # TODO: if sender PID is not valid or (forwarder is running but times are not fresh), kill and restart sender
    pass

def main():
    print "Hello watchdog!"
    # TODO: run 3 threads with the methods above

if __name__ == '__main__':
    main()
