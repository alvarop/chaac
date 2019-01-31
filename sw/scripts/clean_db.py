#!/usr/bin/env python
"""
Use db file
"""

import argparse
import os
import collections
import sys
import sqlite3
import time
import json
import time
from datetime import datetime

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


parser = argparse.ArgumentParser()

parser.add_argument("--in_db", required=True, help="Sqlite db")
parser.add_argument("--out_db", help="Sqlite db")

args = parser.parse_args()


con = None
con = sqlite3.connect(args.in_db)

if con is None:
    raise IOError("Unable to open sqlite database")

cur = con.execute("PRAGMA table_info(samples)")
cols = cur.fetchall()
col_names = []
for col in cols:
    col_names.append(col[1])

WXRecord = collections.namedtuple("WXRecord", col_names)


def wx_row_factory(cursor, row):
    return WXRecord(*row)

def round_record(namedtuple):
    data_dict = namedtuple._asdict()
    for key,val in data_dict.items():
        if isinstance(val, float):
            data_dict[key] = round(val, 3)
    return WXRecord(**data_dict)

con.row_factory = wx_row_factory
cur = con.execute(
    """
    SELECT * FROM samples
    """
)
rows = cur.fetchall()

con.close()


con = None
con = sqlite3.connect(args.out_db)

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

for row in rows:
    rounded_row = round_record(row)

    line = []
    for key in data_columns:
        line.append(getattr(rounded_row, key))

    # print(line)
    cur.execute(sql_insert, line)

con.commit()
con.close()
