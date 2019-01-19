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
from crc import crc16
from datetime import datetime
import collections
from chaac_packet import ChaacPacket
import subprocess

HEADER_LEN = 4
CRC_LEN = 2
START_BYTES = 0xaa55

PACKET_TYPE_DATA = 1
PACKET_TYPE_GPS = 2
PACKET_TYPE_CMD = 3

PacketHeader = ChaacPacket("PacketHeader", [("uid", "I"), ("packet_type", "B")])

WeatherPacket = ChaacPacket(
    "WeatherPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("wind_speed", "f"),
        ("wind_dir", "f"),
        ("rain", "f"),
        ("temperature", "f"),
        ("humidity", "f"),
        ("temperature_in", "f"),
        ("pressure", "f"),
        ("light", "f"),
        ("battery", "f"),
    ],
)

GPSPacket = ChaacPacket(
    "GPSPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("lat_degrees", "i"),
        ("lat_minutes", "d"),
        ("lat_cardinal", "c"),
        ("lon_degrees", "i"),
        ("lon_minutes", "d"),
        ("lon_cardinal", "c"),
    ],
)

CMDPacket = ChaacPacket("CMDPacket", [("uid", "I"), ("packet_type", "B"), ("cmd", "B")])


devices = {}


def check_crc(packet):
    packet_crc = struct.unpack("H", packet[-CRC_LEN:])[0]
    computed_crc = crc16(packet[0:-CRC_LEN])
    return packet_crc == computed_crc


# Encode serial packet with CRC
def encode_packet(data):
    buff = bytearray(len(data) + HEADER_LEN + CRC_LEN)
    struct.pack_into("HH", buff, 0, START_BYTES, len(data))
    struct.pack_into("{}s".format(len(data)), buff, HEADER_LEN, data)
    crc = crc16(buff[0:-CRC_LEN])
    struct.pack_into("H", buff, HEADER_LEN + len(data), crc)

    return buff


def process_packet(packet):

    try:
        packet_bytes = packet[HEADER_LEN:-CRC_LEN]
        header = PacketHeader.decode(packet_bytes)

        if header.packet_type == PACKET_TYPE_DATA:
            print("Sending reset command to {:08X}".format(header.uid))
            data = CMDPacket.encode((header.uid, PACKET_TYPE_CMD, 0xAA))
            stream.write(encode_packet(data))
            stream.flushOutput()
            stream.close()

    except UnicodeDecodeError:
        print("unicode error")
        pass


def decode(buff):

    # Look for start bytes in packet
    for offset in range(len(buff)):

        # Need at least 4 bytes for the header and 2 for crc
        if (len(buff) - offset) < (HEADER_LEN + CRC_LEN):
            return False

        start, dlen = struct.unpack_from("HH", buff, offset)
        if start == START_BYTES:

            # Make sure we have enough bytes for the packet
            if (len(buff) - offset) < (HEADER_LEN + dlen + CRC_LEN):
                return False
            else:
                if (
                    check_crc(buff[offset : (offset + dlen + HEADER_LEN + CRC_LEN)])
                    is True
                ):
                    process_packet(
                        buff[offset : (offset + dlen + HEADER_LEN + CRC_LEN)]
                    )

                    # Remove all data before the packet
                    del buff[: (offset + dlen + HEADER_LEN + CRC_LEN)]
                    return True
                else:
                    # CRC Error, remove the header and keep processing
                    del buff[: (offset + 2)]
                    return True


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
        while decode(buff) is True:
            pass
if args.img:
    time.sleep(1)
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
