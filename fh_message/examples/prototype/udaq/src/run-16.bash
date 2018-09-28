#!/bin/bash
# -------------------------------------------------------------------
# Simulate 16 udaqs in a single process
# -------------------------------------------------------------------

RATE=500
./udaq  \
-u "-d 0x1 -r ${RATE} -p 6660" \
-u "-d 0x2 -r ${RATE} -p 6661" \
-u "-d 0x3 -r ${RATE} -p 6662" \
-u "-d 0x4 -r ${RATE} -p 6663" \
-u "-d 0x5 -r ${RATE} -p 6664" \
-u "-d 0x6 -r ${RATE} -p 6665" \
-u "-d 0x7 -r ${RATE} -p 6666" \
-u "-d 0x8 -r ${RATE} -p 6667" \
-u "-d 0x9 -r ${RATE} -p 6668" \
-u "-d 0x10 -r ${RATE} -p 6669" \
-u "-d 0x11 -r ${RATE} -p 6670" \
-u "-d 0x12 -r ${RATE} -p 6671" \
-u "-d 0x13 -r ${RATE} -p 6672" \
-u "-d 0x14 -r ${RATE} -p 6673" \
-u "-d 0x15 -r ${RATE} -p 6674" \
-u "-d 0x16 -r ${RATE} -p 6675" \
