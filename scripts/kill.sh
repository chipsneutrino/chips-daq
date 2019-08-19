#!/bin/bash

echo "Killing DAQ processes ..."
pkill fsm
pkill daqontrol
pkill daqsitter
pkill daqonite

echo "Unlinking nng files ..."
unlink /tmp/chips_ops.ipc
unlink /tmp/chips_control.ipc
unlink /tmp/chips_daqontrol.ipc
unlink /tmp/chips_daqsitter.ipc
unlink /tmp/chips_daqonite.ipc

echo "Deleting log file ..."
rm apps/log.txt