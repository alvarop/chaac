#!/bin/bash

TOOLS_DIR=`git rev-parse --show-toplevel`/fw/chaac_new/tools

set -m
openocd -f $TOOLS_DIR/st_nucleo_l4.cfg -c init -c halt > /dev/null 2>&1 &
openocd_pid=$!
set +m

sleep 1

arm-none-eabi-gdb -ex "target extended-remote localhost:3333" $1

kill -9 $openocd_pid
