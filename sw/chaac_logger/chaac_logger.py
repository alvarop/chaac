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
from chaac.chaacdb import ChaacDB
from serial_packet.serial_packet import decode_packet, encode_packet

parser = argparse.ArgumentParser()

parser.add_argument("--port", help="device to connect to")

parser.add_argument("--db", help="Sqlite db file")

args = parser.parse_args()

if args.db:
    db = ChaacDB(args.db)
else:
    db = None

if args.port:
    port = args.port
else:
    port = os.environ.get('SERIAL_PORT')

if port is None:
    raise ValueError("Invalid serial port!")

stream = serial.Serial(port, timeout=0.01)
stream.flushInput()

def print_memfault_data(packet):
    memfaultPacket = packets.MemfaultPacket.decode(packet)
    dataLen = memfaultPacket.len
    data = packet[packets.MemfaultPacket.size():(packets.MemfaultPacket.size() + dataLen)]

    print("Memfault data received")
    cmd_str = "--device-serial {:08X} --encoding hex ".format(memfaultPacket.uid)
    for byte in data:
        cmd_str += "{:02X}".format(byte)
    print(cmd_str)

def process_packet(packet):

    header_dict = packets.PacketHeader.decode(packet)._asdict()
    if header_dict["packet_type"] not in packets.PacketTypes:
        print(f"Unknown packet type {header_dict}\n{packet}")
        return

    if packets.PacketTypes[header_dict["packet_type"]] == packets.MemfaultPacket:
        print_memfault_data(packet)
        return

    if header_dict["packet_type"] not in packets.WeatherPacketTypes:
        print("Not a weather packet, ignoring")
        return

    weatherPacket = packets.WeatherPacketTypes[header_dict["packet_type"]]

    packet_dict = weatherPacket.decode(packet)._asdict()

    rxinfo = packets.LoraRxInfo.decode(packet, weatherPacket.size())
    print(rxinfo)

    data = {}
    for key, value in packet_dict.items():
        if key == "wind_dir":
            data[key] = value * 360.0 / 16
        elif key == "wind_dir_deg":
            data["wind_dir"] = value / 10.0
        elif key == "rain":
            data[key] = round(value * 0.2794, 4)
        elif key == "wind_speed" or key == "gust_speed":
            data[key] = value / 100.0
        elif key == "temperature" or key == "alt_temperature" or key == "humidity":
            data[key] = value / 100.0
        elif key == "pressure":
            data[key] = (value + 100000.0) / 100.0
        elif key == "battery" or key == "solar_panel":
            data[key] = value / 1000.0
        else:
            data[key] = packet_dict[key]
    data = weatherPacket.from_dict(data)

    print(data)
    if db is not None:
        db.add_record(data)


buff = bytearray()

while True:
    line = stream.read(1)
    if len(line) > 0:
        buff.append(line[0])
        while decode_packet(buff, process_packet) is True:
            pass
