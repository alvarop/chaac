#!/usr/bin/env python
"""
Decode incoming packets from weather station
"""

import argparse
import os
import struct
import serial
import sys
import time
from crc import crc16
import collections
from chaac_packet import ChaacPacket

HEADER_LEN = 4
CRC_LEN = 2
START_BYTES = 0xaa55

CMDPacket = ChaacPacket(
    "CMDPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("cmd", "B"),
    ],
)

# Encode serial packet with CRC
def encode_packet(data):
    buff = bytearray(len(data)+HEADER_LEN+CRC_LEN)
    struct.pack_into("HH", buff, 0, START_BYTES, len(data))
    struct.pack_into("{}s".format(len(data)), buff, HEADER_LEN, data)
    crc = crc16(buff[0:-CRC_LEN])
    struct.pack_into("H",buff,HEADER_LEN+len(data),crc)

    return buff


parser = argparse.ArgumentParser()

parser.add_argument("--baud_rate", default=115200, type=int, help="xbee baud rate")

parser.add_argument("--port", required=True, help="xbee device to connect to")

args = parser.parse_args()

stream = serial.Serial(args.port, baudrate=args.baud_rate, timeout=0.01)
stream.flushInput()

data = CMDPacket.encode((123,3,0xAA))

stream.write(encode_packet(data))



