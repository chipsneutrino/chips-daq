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

SEC2NOVA = 64e6 # 1s contains 64M NOvA ticks
FSM_REPORTER_INTERVAL = 1
MONITORED_SIGNALS = {
    0: 2 * SEC2NOVA,
    3: 1 * SEC2NOVA
}

###

g_observed_spills = 0
g_observed_spill_times = {}
g_observed_spill_times_lock = threading.Lock()

g_signalled = False
g_stopping_event = threading.Event()
g_stopping = False

def signal_handler(sig_number, frame):
    global g_signalled
    global g_stopping

    if not g_signalled:
        print('Got signal %d, shutting down gracefully. Signal again for immediate exit.' % sig_number)
        g_signalled = True
        g_stopping = True
        g_stopping_event.set()
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
        global g_observed_spill_times_lock
        global g_observed_spill_times
        global g_observed_spills

        if method != 'Spill':
            print('WARNING: Received unusual method (expected "Spill"): %s' % method)
            return
        elif len(params) != 2:
            print('WARNING: Received unusual parameters (expected 2): %s' % params)
            return

        nova_time, spill_type = params
        
        g_observed_spills += 1
        with g_observed_spill_times_lock:
            g_observed_spill_times[int(spill_type)] = int(nova_time)


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
    global g_stopping
    global g_stopping_event
    global g_observed_spill_times_lock
    global g_observed_spill_times
    global g_observed_spills

    local_spill_times = {}
    local_spills = 0

    stale_signals = {signal_type: -1 for signal_type in MONITORED_SIGNALS}

    print('FSM reporter starting with monitoring interval %.f s.' % FSM_REPORTER_INTERVAL)
    while not g_stopping:
        new_local_spills = g_observed_spills
        if new_local_spills > local_spills:
            print('Received %d spill signals in the last monitoring period.' % (new_local_spills - local_spills))

            with g_observed_spill_times_lock:
                new_local_spill_times = g_observed_spill_times.copy()

            stale_signals = {}
            for signal_type in MONITORED_SIGNALS:
                if signal_type not in new_local_spill_times:
                    # signal was never seen before
                    stale_signals[signal_type] = -1
                    continue
                elif signal_type not in local_spill_times:
                    # signal has been seen only recently, don't have anything to compare against
                    stale_signals[signal_type] = -2
                    continue
                
                time_diff = new_local_spill_times[signal_type] - local_spill_times[signal_type]
                if time_diff > MONITORED_SIGNALS[signal_type]:
                    stale_signals[signal_type] = time_diff

            local_spill_times = new_local_spill_times

        # TODO: check that sender PID on PPC is valid
        # TODO: check that forwarder is running
        # TODO: send heartbeat through NNG to FSM
        # TODO: if sender PID is not valid or (forwarder is running but times are not fresh), kill and restart sender
        
        if len(stale_signals) > 0:
            print('WARNING: Found %d stale signals: %s' % (len(stale_signals), list(stale_signals.keys())))

        local_spills = new_local_spills
        g_stopping_event.wait(FSM_REPORTER_INTERVAL)

    print('FSM reporter done.')

def main():
    threads = []
    #threads.append(threading.Thread(target=run_forwarder))
    threads.append(threading.Thread(target=run_loopback_receiver))
    threads.append(threading.Thread(target=run_fsm_reporter))

    for thread in threads:
        thread.start()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    for thread in threads:
        thread.join()

    print('Done.')

if __name__ == '__main__':
    main()
