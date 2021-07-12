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

parser = argparse.ArgumentParser()

parser.add_argument("--port", help="device to connect to")
parser.add_argument("uid", help="uid to reset")

args = parser.parse_args()

if args.port:
    port = args.port
else:
    port = os.environ.get('SERIAL_PORT')

if port is None:
    raise ValueError("Invalid serial port!")

stream = serial.Serial(port, timeout=0.01)
stream.flushInput()

def process_packet(packet):
    header_dict = packets.PacketHeader.decode(packet)._asdict()
    if header_dict["packet_type"] not in packets.PacketTypes:
        print(f"Unknown packet type {header_dict}\n{packet}")
        return

    weatherPacket = packets.PacketTypes[header_dict["packet_type"]]

    packet_dict = weatherPacket.decode(packet)._asdict()

    rxinfo = packets.LoraRxInfo.decode(packet, weatherPacket.size())
    print(rxinfo)
    if packet_dict["uid"] == int(args.uid):
        time.sleep(0.1) # Need delay before responding
        resetPacket = packets.ResetPacket.from_dict({"uid":int(args.uid),"packet_type":1});
        print("Sending Reset Packet")
        print(encode_packet(packets.ResetPacket.encode(resetPacket)))
        stream.write(encode_packet(packets.ResetPacket.encode(resetPacket)));
        sys.exit(0);

buff = bytearray()

print("Waiting for message from {}".format(int(args.uid)))

while True:
    line = stream.read(1)
    if len(line) > 0:
        buff.append(line[0])
        while decode_packet(buff, process_packet) is True:
            pass
