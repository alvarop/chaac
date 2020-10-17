#!/usr/bin/env python
"""
Downsample old db file
"""

import argparse
import os

import collections

import sys
import time
import json
from chaac.chaacdb import ChaacDB
WXRecord = collections.namedtuple("WXRecord", [ "id", "timestamp",
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
    "solar_panel"])

parser = argparse.ArgumentParser()

parser.add_argument("--in_db", required=True, help="Sqlite db")
parser.add_argument("--out_db", help="Sqlite db")

args = parser.parse_args()

in_db = ChaacDB(args.in_db)

if args.out_db:
    out_db = ChaacDB(args.out_db)

rows = in_db.get_records("day")
in_db.close()
idx = 1
for row in rows:
    row = row._asdict()
    row["id"] = idx
    if row["pressure"] > 1500:
        row["pressure"] = int((row["pressure"] * 100) - 100000.0)
        if row["pressure"] & 0x8000:
            row["pressure"] =  (row["pressure"] - 0x10000)
        row["pressure"] = (float(row["pressure"]) + 100000.0)/100.0
    row = WXRecord(**row)
    if args.out_db:
        out_db.add_record(row, row.timestamp, commit=False)
    idx += 1

if args.out_db:
    out_db.close()
