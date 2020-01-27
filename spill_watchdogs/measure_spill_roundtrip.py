#!/usr/bin/env python3

import sys
import subprocess
import time
import threading
import signal
from xmlrpc.server import SimpleXMLRPCServer, SimpleXMLRPCRequestHandler
from paramiko import SSHClient, BadHostKeyException, AuthenticationException, SSHException

kNuMI = 0  # MIBS $74 proton extraction into NuMI
kBNB = 1  # $1B paratisitic beam inhibit
kNuMItclk = 2  # tevatron clock, either $A9 or $AD depending on xml parameter
# booster extraction, $1F (possibly sequence with $1D depending on configuration
kBNBtclk = 3
kAccelOneHztclk = 4  # $8F 1 Hz clock
kFake = 5  # assigned if there is a parity error
kTestConnection = 6
kSuperCycle = 7  # $00, Super cycle and master clock reset
kNuMISampleTrig = 8  # $A4,NuMI cycle sample trigger, reference for $A5
kNuMIReset = 9  # $A5, NuMI reset for beam
kTBSpill = 10  # $39, start of testbeam slow extraction
kTBTrig = 11  # testbeam trigger card signal
kNSpillType = 12  # needs to be at the end, is used for range checking

# TODO: configure from environment
TUNNEL_HOSTNAME = 'localhost'
TUNNEL_PORT = 55812
DISPATCHER_REPORTING_INTERVAL = 20  # spill

WR_ADDR = '192.168.11.7'
WR_USER = 'root'
WR_PASSWORD = ''
WR_APP_PROC_NAME = '/wr/bin/wr_date get | head -n1 | cut -f1 -d\' \''
WR_APP_TIMEOUT = 2
WR_POLLER_FREQ = 0.1  # seconds
WR_POLLER_REPORTING_INTERVAL = 20  # polls

###

g_signalled = False
g_stopping_event = threading.Event()
g_stopping = False

g_latest_time_key = None
g_times = {}


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


class SpillDispatcher():
    def __init__(self):
        self.spill_file = open('./spills', 'w')
        self.spill_count = 0

    def __del__(self):
        self.spill_file.close()

    def get_wr_time(self):
        global g_times
        global g_latest_time_key

        if g_latest_time_key is None:
            return (None, None)

        now = time.time()
        t_wr, t_local = g_times[g_latest_time_key]
        diff = now - t_local
        return (t_wr, diff)

    def _dispatch(self, method, params):
        if method != 'Spill':
            print('WARNING: Received unusual method (expected "Spill"): %s' % method)
            return
        elif len(params) != 2:
            print('WARNING: Received unusual parameters (expected 2): %s' % params)
            return

        wr_time_tai, wr_time_offset = self.get_wr_time()
        nova_time, spill_type = params
        self.spill_file.write(
            f'{spill_type}\t{nova_time}\t{wr_time_tai}\t{wr_time_offset}\n')

        self.spill_count += 1
        if self.spill_count % DISPATCHER_REPORTING_INTERVAL == 0:
            print('Received %d spills.' % self.spill_count)
            self.spill_file.flush()


def run_spill_receiver():
    global g_stopping

    print('Spill receiver starting at %s:%d' %
          (TUNNEL_HOSTNAME, TUNNEL_PORT))
    server = SimpleXMLRPCServer(
        (TUNNEL_HOSTNAME, TUNNEL_PORT), logRequests=False)
    server.timeout = 0.2
    server.register_instance(SpillDispatcher())

    while not g_stopping:
        server.handle_request()

    print('Spill receiver done.')


def run_wr_poller():
    global g_stopping
    global g_times
    global g_latest_time_key

    print('White rabbit poller starting')

    g_times = {}
    g_latest_time_key = None
    poll_count = 0

    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(WR_ADDR, username=WR_USER, password=WR_PASSWORD)

    while not g_stopping:
        wait_time = None
        potential_time_started = time.time()

        if g_latest_time_key is not None:
            latest_wr, latest_local = g_times[g_latest_time_key]
            potential_diff = potential_time_started - latest_local
            potential_wait_time = potential_diff - WR_POLLER_FREQ

            if potential_wait_time > 0:
                wait_time = potential_wait_time

        if wait_time is not None:
            g_stopping_event.wait(wait_time)

        if g_stopping:
            break

        try:
            time_started = time.time()
            stdin, stdout, stderr = ssh.exec_command(
                WR_APP_PROC_NAME, timeout=WR_APP_TIMEOUT)
            lines = [line.strip() for line in stdout.readlines()]

            if len(lines) != 1:
                print('WARNING: Received unusual time response from WR. Out: %s, Err: %s' % (
                    '\n'.join(lines), str(stderr.read())))
            else:
                g_times[poll_count] = (float(lines[0]), time_started)
                g_latest_time_key = poll_count
        except SSHException:
            print('WARNING: Could not connect to WR to get time.')

        poll_count += 1
        if poll_count % WR_POLLER_REPORTING_INTERVAL == 0:
            print('Poller received %d times.' % poll_count)

            old_keys = [key for key in g_times.keys() if poll_count - key > 10]
            for key in old_keys:
                del g_times[key]

    ssh.close()
    print('White rabbit poller done.')


def main():
    threads = []
    threads.append(threading.Thread(target=run_spill_receiver))
    threads.append(threading.Thread(target=run_wr_poller))

    for thread in threads:
        thread.start()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    for thread in threads:
        thread.join()

    print('Done.')


if __name__ == '__main__':
    main()
