#!/bin/bash

TOOLS_DIR=`git rev-parse --show-toplevel`/fw/chaac_new/tools

openocd -f $TOOLS_DIR/st_nucleo_l4.cfg -c "program $1 verify reset exit"
