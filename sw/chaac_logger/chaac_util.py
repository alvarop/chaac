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

    if packets.PacketTypes[header_dict["packet_type"]] == packets.MemfaultPacket:
        print_memfault_data(packet)
        return

    print(header_dict)

    sys.exit(0);

def print_memfault_data(packet):
    memfaultPacket = packets.MemfaultPacket.decode(packet)
    dataLen = memfaultPacket.len
    data = packet[packets.MemfaultPacket.size():(packets.MemfaultPacket.size() + dataLen)]

    print("Memfault data received")
    cmd_str = "--device-serial {:08X} --encoding hex ".format(memfaultPacket.uid)
    for byte in data:
        cmd_str += "{:02X}".format(byte)
    print(cmd_str)

def wait_for_data(timeout):
    start_time = time.time()
    print("Waiting for data\n")
    while True:
        line = stream.read(1)
        if len(line) > 0:
            buff.append(line[0])
            while decode_packet(buff, process_packet) is True:
                pass

        if time.time() - start_time > timeout:
            print("Timeout waiting for data")
            sys.exit(-1)

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

if args.command == "memfault":
    dfuPacket = packets.USBCmdPacket.from_dict({"uid":0,"packet_type":0xE0, "cmd":2 , "len":0});
    print("Sending memfault Packet")
    stream.write(encode_packet(packets.USBCmdPacket.encode(dfuPacket)));
    wait_for_data(2)

if args.command == "ping":
    dfuPacket = packets.USBCmdPacket.from_dict({"uid":0,"packet_type":0xE0, "cmd":3 , "len":0});
    print("Sending ping Packet")
    stream.write(encode_packet(packets.USBCmdPacket.encode(dfuPacket)));

    wait_for_data(2)
    