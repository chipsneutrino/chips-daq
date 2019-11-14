#!/usr/bin/env python2

import subprocess

def run_forwarder():
    # TODO: start forwarder, keep watching over it
    # TODO: restart forwarder if it stops for any reason
    pass

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
