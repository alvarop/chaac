#!/usr/bin/env python
"""
Decode incoming packets from weather station and save to db
"""

import argparse
import os
import serial
import sys
import time
from datetime import datetime
from chaac import packets
from serial_packet.serial_packet import decode_packet, encode_packet



def process_packet(packet):
    header_dict = packets.PacketHeader.decode(packet)._asdict()
    if header_dict["packet_type"] not in packets.PacketTypes:
        print(f"Unknown packet type {header_dict}\n{packet}")
        return

    print(header_dict)

    sys.exit(0);

buff = bytearray()

parser = argparse.ArgumentParser()

parser.add_argument("port", help="device to connect to")
parser.add_argument("command", help="Command")

args = parser.parse_args()

stream = serial.Serial(args.port, timeout=0.01)
stream.flushInput()

if args.command == "reset":
    dfuPacket = packets.USBCmdPacket.from_dict({"uid":0,"packet_type":0xE0, "cmd":0 , "len":0});
    print("Sending Reset Packet")
    stream.write(encode_packet(packets.USBCmdPacket.encode(dfuPacket)));

if args.command == "dfu":
    dfuPacket = packets.USBCmdPacket.from_dict({"uid":0,"packet_type":0xE0, "cmd":1 , "len":0});
    print("Sending DFU Packet")
    stream.write(encode_packet(packets.USBCmdPacket.encode(dfuPacket)));

if args.command == "ping":
    dfuPacket = packets.USBCmdPacket.from_dict({"uid":0,"packet_type":0xE0, "cmd":3 , "len":0});
    print("Sending ping Packet")
    stream.write(encode_packet(packets.USBCmdPacket.encode(dfuPacket)));

    timeout = 2
    start_time = time.time()
    print("Waiting for pong\n")
    while True:
        line = stream.read(1)
        if len(line) > 0:
            buff.append(line[0])
            while decode_packet(buff, process_packet) is True:
                pass

        if time.time() - start_time > timeout:
            print("Timeout waiting for pong")
            sys.exit(-1)