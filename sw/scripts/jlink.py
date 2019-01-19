#!/usr/bin/env python

import argparse
import os
import sys
import time
import tempfile
import subprocess

JLINK_PATH = "/usr/bin/JLinkExe"

parser = argparse.ArgumentParser()

parser.add_argument("--dump", action="store_true")
parser.add_argument("--erase", action="store_true")
parser.add_argument("--device", default="stm32l432kc", help="device name")
parser.add_argument("--addr", default=0, help="start addr")
parser.add_argument("--len", default=0x40000, help="read/write len")
parser.add_argument("--filename", help="filename")

args = parser.parse_args()


def write_line(file, line):

    if line[-1] != "\n":
        line += "\n"
    file.write(line.encode("utf-8"))


with tempfile.NamedTemporaryFile(prefix="jlink", delete=False) as scriptfile:
    write_line(scriptfile, "if swd")
    write_line(scriptfile, "device {}".format(args.device))
    write_line(scriptfile, "speed 4000")
    write_line(scriptfile, "connect")
    write_line(scriptfile, "halt")

    if args.erase == True:
        write_line(
            scriptfile,
            "erase",
        )
    elif args.dump == True:
        write_line(
            scriptfile,
            "savebin {} 0x{:X} 0x{:X}".format(args.filename, args.addr, args.len),
        )

    write_line(scriptfile, "go")
    write_line(scriptfile, "exit")

    command_file = scriptfile.name

if command_file:
    subprocess.call([JLINK_PATH, command_file])
