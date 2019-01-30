#!/usr/bin/env python
"""
Decode incoming packets from weather station
"""

import argparse
import os
import serial
import sys
import time
import collections
from chaac import packets
from serial_packet import encode_packet

parser = argparse.ArgumentParser()

parser.add_argument("--baud_rate", default=115200, type=int, help="xbee baud rate")

parser.add_argument("--port", required=True, help="xbee device to connect to")

args = parser.parse_args()

stream = serial.Serial(args.port, baudrate=args.baud_rate, timeout=0.01)
stream.flushInput()

data = packets.CMDPacket.encode((123, 3, 0xAA))

stream.write(encode_packet(data))
