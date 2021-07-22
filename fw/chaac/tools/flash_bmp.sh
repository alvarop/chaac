#!/bin/bash

TOOLS_DIR=`git rev-parse --show-toplevel`/fw/chaac_new/tools

arm-none-eabi-gdb -nx --batch \
  -ex 'target extended-remote /dev/ttyBmpGdb' \
  -x $TOOLS_DIR/black_magic_probe_flash_swd.scr \
  $1
