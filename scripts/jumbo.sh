#!/bin/bash
##############################################################
# jumbo.sh
# Script to enable jumbo packets on remote machines
##############################################################

# What machines to configure
MACHINES="192.168.11.7 192.168.11.8 192.168.11.101 192.168.11.102 192.168.11.121 192.168.11.122 192.168.11.141 192.168.11.161 192.168.11.181 192.168.11.182"

##############################################################

set -e

enable_jumbo() {
    MACHINE_ADDR=$1 ; shift
    ssh root@${MACHINE_ADDR} "/sbin/devmem 0x10030008 32 0x00800002; /sbin/devmem 0x10030408 32 0x00800002; /sbin/devmem 0x10030808 32 0x00800002; /sbin/devmem 0x10030c08 32 0x00800002; /sbin/devmem 0x10031008 32 0x00800002; /sbin/devmem 0x10031408 32 0x00800002; /sbin/devmem 0x10031808 32 0x00800002; /sbin/devmem 0x10031c08 32 0x00800002; /sbin/devmem 0x10032008 32 0x00800002; /sbin/devmem 0x10032408 32 0x00800002; /sbin/devmem 0x10032808 32 0x00800002; /sbin/devmem 0x10032c08 32 0x00800002; /sbin/devmem 0x10033008 32 0x00800002; /sbin/devmem 0x10033408 32 0x00800002; /sbin/devmem 0x10033808 32 0x00800002; /sbin/devmem 0x10033c08 32 0x00800002; /sbin/devmem 0x10034008 32 0x00800002; /sbin/devmem 0x10034408 32 0x00800002"
}

echo "Will configure machines: ${MACHINES}"

for MACHINE in $MACHINES ; do
    echo "Configure ${MACHINE}"
    enable_jumbo "${MACHINE}"
done

echo "Done."
