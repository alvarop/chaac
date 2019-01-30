#!/usr/bin/env python
"""
Simple serial update script for Chaac.
Listen for incoming packets. Once one is received, send the DFU command
and begin firmware update process. Reset device when done.
"""

import argparse
import os
import struct
import serial
import sys
import time
from datetime import datetime
import collections
from chaac import packets
from serial_packet.serial_packet import decode_packet, encode_packet
import subprocess

devices = {}

def process_packet(packet_bytes):

    try:
        header = packets.PacketHeader.decode(packet_bytes)

        if header.packet_type == packets.PACKET_TYPE_DATA:
            print("Sending reset command to {:08X}".format(header.uid))
            data = packets.CMDPacket.encode((header.uid, packets.PACKET_TYPE_CMD, 0xAA))
            stream.write(encode_packet(data))
            stream.flushOutput()
            stream.close()

    except UnicodeDecodeError:
        print("unicode error")
        pass



parser = argparse.ArgumentParser()

parser.add_argument("--baud_rate", default=115200, type=int, help="xbee baud rate")

parser.add_argument("--port", required=True, help="xbee device to connect to")

parser.add_argument("--img", help="FW image file")

args = parser.parse_args()

stream = serial.Serial(args.port, baudrate=args.baud_rate, timeout=0.01)
stream.flushInput()

buff = bytearray()

while stream.isOpen():
    line = stream.read(1)
    if len(line) > 0:
        buff.append(line[0])
        while decode_packet(buff, process_packet) is True:
            pass
if args.img:
    time.sleep(2)
    print("Loading image...")
    subprocess.call(
        [
            "mcumgr",
            "--conntype",
            "serial",
            "--connstring",
            "dev={},mtu=256".format(args.port),
            "image",
            "upload",
            "-e",
            args.img,
            # "-l",
            # "debug"
        ]
    )

    print("Restarting")
    subprocess.call(
        [
            "mcumgr",
            "--conntype",
            "serial",
            "--connstring",
            "dev={},mtu=256".format(args.port),
            "reset",
            # "-l",
            # "debug"
        ]
    )
