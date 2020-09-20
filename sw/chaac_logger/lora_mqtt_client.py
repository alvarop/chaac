#!/usr/bin/env python
"""
Decode incoming packets from weather station and save to db
"""

import argparse
import os
import serial
import sys
import time
import json
import paho.mqtt.client as mqtt
from datetime import datetime
from chaac import packets
from serial_packet.serial_packet import decode_packet, encode_packet

parser = argparse.ArgumentParser()

parser.add_argument("--baud_rate", default=115200, type=int, help="baud rate")
parser.add_argument("--port", required=True, help="device to connect to")

# TODO - load settings from config file
parser.add_argument("--mqtt_server", required=True, help="MQTT server to connect to")
parser.add_argument("--mqtt_port", type=int, default=1883, help="MQTT server port")

args = parser.parse_args()

stream = serial.Serial(args.port, baudrate=args.baud_rate, timeout=0.01)
stream.flushInput()

last_packet = None

def connect_msg():
    print("Connected to Broker")

def publish_msg():
    print("Message Published")

client = mqtt.Client(client_id="chaac_publisher")

client.on_connect = connect_msg
client.on_publish = publish_msg

# TODO - load from config or use certs
client.username_pw_set("mqtt_user", "tmp_pass")

client.connect(args.mqtt_server, args.mqtt_port, 60)


def process_packet(packet):
    global last_packet

    header_dict = packets.PacketHeader.decode(packet)._asdict()
    packet_dict = packets.WeatherPacketV1P0.decode(packet)._asdict()

    rxinfo = packets.LoraRxInfo.decode(packet, packets.WeatherPacketV1P0.size())
    print(rxinfo)

    # Don't process duplicate packets!
    if (
        last_packet is not None
        and last_packet["uid"] == packet_dict["uid"]
        and last_packet["sample"] == packet_dict["sample"]
    ):
        return

    last_packet = packet_dict

    data = {}
    mqtt_packets = []
    timestamp = int(time.time())
    for key, value in packet_dict.items():
        if key == "wind_dir":
            data[key] = value * 360.0 / 16
            mqtt_packets.append(
                (key, {"timestamp": timestamp, "value": data[key], "unit": "degrees"})
            )
        elif key == "rain":
            data[key] = round(value * 0.2794, 4)
            mqtt_packets.append(
                (key, {"timestamp": timestamp, "value": data[key], "unit": "mm"})
            )
        elif key == "wind_speed":
            data[key] = value / 100.0
            mqtt_packets.append(
                (key, {"timestamp": timestamp, "value": data[key], "unit": "kph"})
            )
        elif key == "temperature":
            data[key] = value / 100.0
            mqtt_packets.append(
                (key, {"timestamp": timestamp, "value": data[key], "unit": "C"})
            )
        elif key == "humidity":
            data[key] = value / 100.0
            mqtt_packets.append(
                (key, {"timestamp": timestamp, "value": data[key], "unit": "%RH"})
            )
        elif key == "pressure":
            data[key] = value + 100000.0
            mqtt_packets.append(
                (key, {"timestamp": timestamp, "value": data[key], "unit": "Pa"})
            )
        elif key == "battery" or key == "solar_panel":
            data[key] = value / 1000.0
            mqtt_packets.append(
                (key, {"timestamp": timestamp, "value": data[key], "unit": "V"})
            )
        else:
            data[key] = packet_dict[key]

    for packet in mqtt_packets:
        client.publish(
            "chaac/{}/{}".format(data["uid"], packet[0]), json.dumps(packet[1])
        )

    data = packets.WeatherPacketV1P0.from_dict(data)
    print(data)


buff = bytearray()

client.loop_start()

while True:
    line = stream.read(1)
    if len(line) > 0:
        buff.append(line[0])
        while decode_packet(buff, process_packet) is True:
            pass

client.loop_stop()
