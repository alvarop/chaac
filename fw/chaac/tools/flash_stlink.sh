#!/bin/bash

TOOLS_DIR=`git rev-parse --show-toplevel`/fw/chaac/tools

openocd -f $TOOLS_DIR/stlink-mod.cfg -f $TOOLS_DIR/st_nucleo_l4_hardreset.cfg -c "program $1 verify reset exit"
