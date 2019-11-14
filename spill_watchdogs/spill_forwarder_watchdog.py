#!/usr/bin/env python3

import subprocess
import time
import threading
from xmlrpc.server import SimpleXMLRPCServer

# TODO: configure from environment
FORWARDER_PROC_NAME = ['NssSpillForwarder', '-c NovaSpillServer/config/SpillForwarderConfig-CHIPS.xml', '-D 0', '-M 0']
FORWARDER_HOLDOFF_SEC = 1

LOOPBACK_HOSTNAME = 'localhost'
LOOPBACK_PORT = 17898

###

g_observed_spill_times = {}
g_observed_spill_times_lock = threading.Lock()

def run_forwarder():
    running_forwarder = True

    while running_forwarder:
        forwarder = subprocess.run(FORWARDER_PROC_NAME)
        # TODO: react on `forwarder.returncode`

        time.sleep(FORWARDER_HOLDOFF_SEC)

def run_loopback_receiver():
    def loopback_spill(nova_time, spill_type):
        with g_observed_spill_times_lock:
            g_observed_spill_times[spill_type] = nova_time
        return 'Ok'

    server = SimpleXMLRPCServer((LOOPBACK_HOSTNAME, LOOPBACK_PORT))
    server.register_function(loopback_spill, 'Spill')
    server.serve_forever()

def run_fsm_reporter():
    # TODO: run on timer
    # TODO: check that sender PID on PPC is valid
    # TODO: check that forwarder is running
    # TODO: check that all loopback times are fresh
    # TODO: send heartbeat through NNG to FSM
    # TODO: if sender PID is not valid or (forwarder is running but times are not fresh), kill and restart sender
    pass

def main():
    threads = []
    threads.append(threading.Thread(target=run_forwarder))
    threads.append(threading.Thread(target=run_loopback_receiver))
    threads.append(threading.Thread(target=run_fsm_reporter))

    for thread in threads:
        thread.start()
        
    for thread in threads:
        thread.join()

if __name__ == '__main__':
    main()
