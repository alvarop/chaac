#!/bin/bash

TOOLS_DIR=`git rev-parse --show-toplevel`/fw/chaac/tools

arm-none-eabi-gdb \
  -ex 'target extended-remote /dev/ttyBmpGdb' \
  -x $TOOLS_DIR/black_magic_probe_debug_swd.scr \
  $1
