#!/usr/bin/env python3

import sys
import subprocess
import time
import threading
import signal
from xmlrpc.server import SimpleXMLRPCServer, SimpleXMLRPCRequestHandler

# TODO: configure from environment
FORWARDER_PROC_NAME = ['NssSpillForwarder', '-c NovaSpillServer/config/SpillForwarderConfig-CHIPS.xml', '-D 0', '-M 0']
FORWARDER_HOLDOFF_SEC = 1

LOOPBACK_HOSTNAME = 'localhost'
LOOPBACK_PORT = 17898

###

g_observed_spill_times = {}
g_observed_spill_times_lock = threading.Lock()

g_signalled = False
g_stopping = False

def signal_handler(sig_number, frame):
    global g_signalled
    global g_stopping

    if not g_signalled:
        g_signalled = True
        g_stopping = True
        print('Got signal %d, shutting down gracefully. Signal again to kill the process immediately.' % sig_number)
    else:
        print('Signalled (%d) again, terminating hard.' % sig_number)
        sys.exit(0)

def run_forwarder():
    running_forwarder = True

    while running_forwarder:
        forwarder = subprocess.run(FORWARDER_PROC_NAME)
        # TODO: react on `forwarder.returncode`

        time.sleep(FORWARDER_HOLDOFF_SEC)

class LoopbackDispatcher():
     def _dispatch(self, method, params):
        if method != 'Spill':
            print('WARNING: Received unusual method (expected "Spill"): %s' % method)
            return
        elif len(params) != 2:
            print('WARNING: Received unusual parameters (expected 2): %s' % params)
            return

        nova_time, spill_type = params
        if not isinstance(nova_time, int) or not isinstance(spill_type, int):
            print('WARNING: Received unusual parameters (expected int, int): %s' % params)
            return
        
        with g_observed_spill_times_lock:
            g_observed_spill_times[spill_type] = nova_time

def run_loopback_receiver():
    global g_stopping

    print('Loopback receiver starting at %s:%d' % (LOOPBACK_HOSTNAME, LOOPBACK_PORT))
    with SimpleXMLRPCServer((LOOPBACK_HOSTNAME, LOOPBACK_PORT), logRequests=False) as server:
        server.timeout = 0.2
        server.register_instance(LoopbackDispatcher())
        
        while not g_stopping:
            server.handle_request()

    print('Loopback receiver done.')

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
    #threads.append(threading.Thread(target=run_forwarder))
    threads.append(threading.Thread(target=run_loopback_receiver))
    #threads.append(threading.Thread(target=run_fsm_reporter))

    for thread in threads:
        thread.start()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    for thread in threads:
        thread.join()

    print('Done.')

if __name__ == '__main__':
    main()
