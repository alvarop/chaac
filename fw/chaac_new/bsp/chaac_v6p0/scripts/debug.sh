#!/bin/bash

#SCRIPTS_DIR=`git rev-parse --show-toplevel`/fw/scripts
SCRIPTS_DIR=./scripts

arm-none-eabi-gdb \
  -ex 'target extended-remote /dev/ttyBmpGdb' \
  -x $SCRIPTS_DIR/black_magic_probe_debug_swd.scr \
  $1
