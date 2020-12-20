#!/bin/bash

#SCRIPTS_DIR=`git rev-parse --show-toplevel`/fw/scripts
SCRIPTS_DIR=./scripts

arm-none-eabi-gdb -nx --batch \
  -ex 'target extended-remote /dev/ttyBmpGdb' \
  -x $SCRIPTS_DIR/black_magic_probe_flash_swd.scr \
  $1
