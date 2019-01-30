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

parser.add_argument("--db", required=True, help="Input dbs", nargs="*")
parser.add_argument("--out_db", help="Output db")

args = parser.parse_args()

rows = []

for db in args.db:

    con = None
    con = sqlite3.connect(db)

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

    con.row_factory = wx_row_factory
    cur = con.execute(
        """
        SELECT * FROM samples
        """
    )
    rows += cur.fetchall()

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

prev_row = None
for row in rows:
    print(row)
    line = []
    for key in data_columns:
        line.append(getattr(row, key))
    print(line)

    cur.execute(sql_insert, line)

con.commit()
con.close()
