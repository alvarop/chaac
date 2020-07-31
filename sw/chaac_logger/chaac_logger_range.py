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

args = parser.parse_args()

stream = serial.Serial(args.port, baudrate=args.baud_rate, timeout=0.01)
stream.flushInput()

def process_packet(packet):
    global last_packet
    
    header = packets.PacketHeader.decode(packet)._asdict()
    if header["packet_type"] != 0xF0:
        # print("Unknown packet type {:02X}".format(header["packet_type"]))
        return
    packet_dict = packets.RangeTestPacketV1P0.decode(packet)._asdict()
    rxinfo = packets.LoraRxInfo.decode(packet, packets.RangeTestPacketV1P0.size())

    line = []

    line.append(str(datetime.now().isoformat()))
    line.append("{:08X}".format(packet_dict["uid"]))
    line.append(str(packet_dict["sample"]))
    line.append(str(packet_dict["tx_pwr"]))
    line.append(str(rxinfo.rssi))
    line.append(str(rxinfo.snr))

    print(",".join(line))

buff = bytearray()

header = ["timestamp", "uid","sample","tx_pwr","rssi","snr"]
print(",".join(header))

while True:
    line = stream.read(1)
    if len(line) > 0:
        buff.append(line[0])
        while decode_packet(buff, process_packet) is True:
            pass
