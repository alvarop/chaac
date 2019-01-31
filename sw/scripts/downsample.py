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
from statistics import mean

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
    # "solar_voltage"
]

sql_insert_day = "INSERT INTO day_samples VALUES(NULL,{})".format(
    ",".join(["?"] * len(data_columns))
)

sql_insert_week = "INSERT INTO week_samples VALUES(NULL,{})".format(
    ",".join(["?"] * len(data_columns))
)

sql_insert_month = "INSERT INTO month_samples VALUES(NULL,{})".format(
    ",".join(["?"] * len(data_columns))
)

parser = argparse.ArgumentParser()

parser.add_argument("--in_db", required=True, help="Sqlite db")
parser.add_argument("--out_db", help="Sqlite db")

args = parser.parse_args()


con_in = None
con_in = sqlite3.connect(args.in_db)

if con_in is None:
    raise IOError("Unable to open sqlite database")

cur_in = con_in.execute("PRAGMA table_info(samples)")
cols = cur_in.fetchall()
col_names = []
for col in cols:
    col_names.append(col[1])

WXRecord = collections.namedtuple("WXRecord", col_names)


def wx_row_factory(cursor, row):
    return WXRecord(*row)


con_out = None
con_out = sqlite3.connect(args.out_db)

if con_out is None:
    raise IOError("Unable to open sqlite database")

cur_out = con_out.cursor()
cur_out.execute(
    "CREATE TABLE IF NOT EXISTS "
    + "day_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
    + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
)

cur_out.execute(
    "CREATE TABLE IF NOT EXISTS "
    + "week_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
    + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
)

cur_out.execute(
    "CREATE TABLE IF NOT EXISTS "
    + "month_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
    + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
)

cur_out.execute(
    "CREATE TABLE IF NOT EXISTS "
    + "devices(uid INTEGER PRIMARY KEY, name TEXT, gps TEXT)"
)


con_in.row_factory = wx_row_factory
start_date = int(datetime(2018, 12, 20, 0, 0).timestamp())
end_date = int(time.time())


print("Days")
query = """
        SELECT * FROM samples
        """

cur_in = con_in.execute(query)
rows = cur_in.fetchall()

for row in rows:
    line = []
    for key in data_columns:
        line.append(getattr(row, key))

    # print(line)
    cur_out.execute(sql_insert_day, line)

print("Weeks")
avg_time_delta = 60 * 7
for date in range(start_date, end_date, avg_time_delta):
    start_time = date
    end_time = start_time + avg_time_delta
    query = """
        SELECT * FROM samples
        WHERE timestamp > {}
        AND timestamp < {}
        ORDER BY timestamp
        """.format(
        int(start_time), int(end_time)
    )

    cur_in = con_in.execute(query)
    rows = cur_in.fetchall()

    if len(rows) == 0:
        continue
    lines = []
    for row in rows:
        line = []
        for key in data_columns:
            line.append(getattr(row, key))
        lines.append(line)

        # Add "solar_voltage" for future use
        # line.append(0.0)

    avg_line = list(map(mean, zip(*lines)))
    avg_line[data_columns.index("timestamp")] = int(avg_line[data_columns.index("timestamp")])  # Round out the timestamp to seconds
    avg_line[data_columns.index("uid")] = lines[0][data_columns.index("uid")]  # Don't average the device number!

    # Round the rest of the items
    for idx in range(2, len(avg_line)):
        avg_line[idx] = round(avg_line[idx], 3)

    # Oh wait, except for rain! Add that one up...
    avg_line[data_columns.index("rain")] = 0
    for line in lines:
        avg_line[data_columns.index("rain")] += line[data_columns.index("rain")]
    avg_line[data_columns.index("rain")] = round(avg_line[data_columns.index("rain")],3)

    cur_out.execute(sql_insert_week, avg_line)

print("Months")
avg_time_delta = 60 * 31
for date in range(start_date, end_date, avg_time_delta):
    start_time = date
    end_time = start_time + avg_time_delta
    query = """
        SELECT * FROM samples
        WHERE timestamp > {}
        AND timestamp < {}
        ORDER BY timestamp
        """.format(
        int(start_time), int(end_time)
    )

    cur_in = con_in.execute(query)
    rows = cur_in.fetchall()

    if len(rows) == 0:
        continue
    lines = []
    for row in rows:
        line = []
        for key in data_columns:
            line.append(getattr(row, key))
        lines.append(line)

        # Add "solar_voltage" for future use
        # line.append(0.0)

    avg_line = list(map(mean, zip(*lines)))
    avg_line[0] = int(avg_line[0])  # Round out the timestamp to seconds
    avg_line[1] = lines[0][1]  # Don't average the device number!
    for idx in range(2, len(avg_line)):
        avg_line[idx] = round(avg_line[idx], 3)

    cur_out.execute(sql_insert_month, avg_line)

con_out.commit()
con_out.close()

con_in.close()
