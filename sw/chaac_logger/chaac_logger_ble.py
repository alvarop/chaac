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

parser.add_argument("--baud_rate", default=115200, type=int, help="baud rate")

parser.add_argument("--port", required=True, help="device to connect to")

parser.add_argument("--db", help="Sqlite db file")

args = parser.parse_args()

if args.db:
    db = ChaacDB(args.db)
else:
    db = None

stream = serial.Serial(args.port, baudrate=args.baud_rate, timeout=0.01)
stream.flushInput()

last_packet = None


def process_packet(packet):
    global last_packet
    packet_dict = packets.BLEWeatherPacket.decode(packet)._asdict()

    # Don't process duplicate packets!
    if (
        last_packet is not None
        and last_packet["uid"] == last_packet["uid"]
        and last_packet["sample"] == packet_dict["sample"]
    ):
        return

    last_packet = packet_dict

    data = {}
    for key, value in packet_dict.items():
        if key == "wind_dir":
            data[key] = value * 360.0 / 16
        elif key == "rain":
            data[key] = value * 0.2794
        elif key == "wind_speed":
            data[key] = value / 100.0
        elif key == "temperature" or key == "humidity":
            data[key] = value / 100.0
        elif key == "pressure":
            data[key] = (value + 100000.0) / 100.0
        elif key == "battery" or key == "solar_panel":
            data[key] = value / 1000.0
        else:
            data[key] = packet_dict[key]
    data = packets.BLEWeatherPacket.from_dict(data)

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
