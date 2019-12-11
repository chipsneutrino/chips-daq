#!/bin/bash

source config.sh

# TODO: rename to CHIPS_ELASTIC_CLIENT in programs
export ELASTIC_CLIENT="${CHIPS_ELASTIC_CLIENT}"

exec $@

