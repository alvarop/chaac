#!/usr/bin/env python
"""
Decode incoming packets from weather station and save to db
"""

import argparse
import os
import serial
import sys
import sqlite3
import time
from datetime import datetime
import collections
from chaac import packets
from serial_packet.serial_packet import decode_packet

# timestamp and uid must be the first two
data_columns = [
    "timestamp",
    "uid",
    "temperature",
    "humidity",
    "pressure",
    "temperature_in",
    "light",
    "battery",
    "rain",
    "wind_speed",
    "wind_dir",
]


sql_insert = "INSERT INTO samples VALUES(NULL,{})".format(
    ",".join(["?"] * len(data_columns))
)

devices = {}

def process_data_packet(packet):
    data = packets.WeatherPacket.decode(packet)
    data = packets.WeatherPacket.round(data)
    print(data)

    if args.db:
        save_sqlite_data(data)


def process_gps_packet(packet):
    data = packets.GPSPacket.decode(packet)
    print(data)

    if args.db:
        # TODO - only add this if not set
        add_sqlite_device(data.uid, None, get_gps_string(data))


packet_processors = {
    packets.PACKET_TYPE_DATA: process_data_packet,
    packets.PACKET_TYPE_GPS: process_gps_packet,
}


def get_gps_string(gps_data):
    gps_str = "{} {} {} {} {} {}".format(
        gps_data.lat_degrees,
        gps_data.lat_minutes,
        gps_data.lat_cardinal,
        gps_data.lon_degrees,
        gps_data.lon_minutes,
        gps_data.lon_cardinal,
    )

    return gps_str


def read_sqlite_devices():
    query = "select * from devices"
    cur.execute(query)
    rows = cur.fetchall()

    for row in rows:
        devices[row[0]] = row[1]


def add_sqlite_device(uid, name, gps_str):

    if name is None:
        name = devices[uid]

    sql_query = 'REPLACE INTO devices (uid, name, gps) VALUES ({}, "{}", "{}");'.format(
        uid, name, gps_str
    )

    cur.execute(sql_query)

    # Add retries in case the database is locked
    retries = 5
    while retries > 0:
        try:
            con.commit()
            break
        except sqlite3.OperationalError:
            print("Unable to commit. Retrying {}".format(retries))
            retries -= 1
            continue

    devices[uid] = name


def save_sqlite_data(data):

    if data.uid not in devices:
        print("New device! {}".format(data.uid))
        add_sqlite_device(data.uid, str(data.uid), "")

    line = [int(time.time())]

    for key in data_columns:
        if key != "timestamp":
            line.append(getattr(data, key))

    cur.execute(sql_insert, line)

    # Add retries in case the database is locked
    retries = 5
    while retries > 0:
        try:
            con.commit()
            break
        except sqlite3.OperationalError:
            print("Unable to commit. Retrying {}".format(retries))
            retries -= 1
            continue


def process_packet(packet_bytes):

    try:
        header = packets.PacketHeader.decode(packet_bytes)

        if header.packet_type in packet_processors:
            packet_processors[header.packet_type](packet_bytes)
        else:
            print("ERR: Unknown type", header.packet_type)

    except UnicodeDecodeError:
        print("unicode error")
        pass


parser = argparse.ArgumentParser()

parser.add_argument("--baud_rate", default=115200, type=int, help="xbee baud rate")

parser.add_argument("--port", required=True, help="xbee device to connect to")

parser.add_argument("--db", help="Sqlite db file")

args = parser.parse_args()

if args.db:
    con = None
    con = sqlite3.connect(args.db)

    if con is None:
        raise IOError("Unable to open sqlite database")

    cur = con.cursor()
    cur.execute(
        "CREATE TABLE IF NOT EXISTS "
        + "samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
        + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
    )

    cur.execute(
        "CREATE TABLE IF NOT EXISTS "
        + "devices(uid INTEGER PRIMARY KEY, name TEXT, gps TEXT)"
    )

    read_sqlite_devices()

stream = serial.Serial(args.port, baudrate=args.baud_rate, timeout=0.01)
stream.flushInput()

buff = bytearray()

while True:
    line = stream.read(1)
    if len(line) > 0:
        buff.append(line[0])
        while decode_packet(buff, process_packet) is True:
            pass
