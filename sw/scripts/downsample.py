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

parser = argparse.ArgumentParser()

parser.add_argument("--in_db", required=True, help="Sqlite db")
parser.add_argument("--out_db", help="Sqlite db")

args = parser.parse_args()


class ChaacDB:
    def __init__(self, filename):
        self.conn = sqlite3.connect(filename)

        if self.conn is None:
            raise IOError("Unable to open sqlite database")

        self.WXRecord = collections.namedtuple("WXRecord", ['id'] + data_columns)

        self.tables = {
        'day': 'day_samples',
        'week': 'week_samples',
        'month': 'month_samples',
        }

        # Table to store daily data (every minute)
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "day_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
            + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
        )

        # Table to store weekly data (7 minute average)
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "week_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
            + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
        )

        # Table to store monthly data (31 minute average)
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "month_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
            + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
        )

        # Table to store devices and locations (if available)
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "devices(uid INTEGER PRIMARY KEY, name TEXT, gps TEXT)"
        )

        # Table to store configuration key/value pairs
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "config(key TEXT UNIQUE, value TEXT)"
        )

        self.cur = self.conn.cursor()

    def close(self):
        self.conn.close()

    def wx_row_factory(self, cursor, row):
        return self.WXRecord(*row)

    def get_records(self, table, limit=None, start_date=None, end_date=None):
        if table not in self.tables:
            raise KeyError("Invalid table!")

        self.cur.row_factory = self.wx_row_factory

        query = "SELECT * FROM {}".format(self.tables[table])

        if limit is not None:
            query += " LIMIT {}".format(int(limit))

        if start_date is not None:
            query += " WHERE timestamp >= {}".format(int(start_date))

        # TODO - handle end data with no start date
        if end_date is not None:
            query += " AND timestamp >= {}".format(int(end_date))

        self.cur.execute(query)
        return self.cur.fetchall()

    def __insert_line(self, line):
        table = self.tables['day']
        query = "INSERT INTO {} VALUES(NULL,{})".format(
            table,
            ",".join(["?"] * len(data_columns))
        )

        self.cur.execute(query, line)

        # TODO - figure out downsampling here

    def __commit(self):
        retries = 5
        while retries > 0:
            try:
                self.conn.commit()
                break
            except sqlite3.OperationalError:
                retries -= 1
                continue

    def add_record(self, record, timestamp=None):
        # If timestamp is none, use current time
        if timestamp is None:
            timestamp = int(time.time())

        line = [timestamp]

        for key in data_columns:
            if key != "timestamp":
                line.append(getattr(record, key))

        self.__insert_line(line)
        # self.__commit();

        print(line)
        

in_db = ChaacDB(args.in_db)

if args.out_db:
    out_db = ChaacDB(args.out_db)

rows = in_db.get_records('day')
in_db.close()
for row in rows:
    print(row)
    if args.out_db:
        out_db.add_record(row, row.timestamp)



# con_out = None
# con_out = sqlite3.connect(args.out_db)

# if con_out is None:
#     raise IOError("Unable to open sqlite database")

# cur_out = con_out.cursor()


# con_in.row_factory = wx_row_factory
# start_date = int(datetime(2018, 12, 20, 0, 0).timestamp())
# end_date = int(time.time())


# print("Days")
# query = """
#         SELECT * FROM samples
#         """

# cur_in = con_in.execute(query)
# rows = cur_in.fetchall()

# for row in rows:
#     line = []
#     for key in data_columns:
#         line.append(getattr(row, key))

#     # print(line)
#     cur_out.execute(sql_insert_day, line)

# print("Weeks")
# avg_time_delta = 60 * 7
# for date in range(start_date, end_date, avg_time_delta):
#     start_time = date
#     end_time = start_time + avg_time_delta
#     query = """
#         SELECT * FROM samples
#         WHERE timestamp > {}
#         AND timestamp < {}
#         ORDER BY timestamp
#         """.format(
#         int(start_time), int(end_time)
#     )

#     cur_in = con_in.execute(query)
#     rows = cur_in.fetchall()

#     if len(rows) == 0:
#         continue
#     lines = []
#     for row in rows:
#         line = []
#         for key in data_columns:
#             line.append(getattr(row, key))
#         lines.append(line)

#         # Add "solar_voltage" for future use
#         # line.append(0.0)

#     avg_line = list(map(mean, zip(*lines)))
#     avg_line[data_columns.index("timestamp")] = int(
#         avg_line[data_columns.index("timestamp")]
#     )  # Round out the timestamp to seconds
#     avg_line[data_columns.index("uid")] = lines[0][
#         data_columns.index("uid")
#     ]  # Don't average the device number!

#     # Round the rest of the items
#     for idx in range(2, len(avg_line)):
#         avg_line[idx] = round(avg_line[idx], 3)

#     # Oh wait, except for rain! Add that one up...
#     avg_line[data_columns.index("rain")] = 0
#     for line in lines:
#         avg_line[data_columns.index("rain")] += line[data_columns.index("rain")]
#     avg_line[data_columns.index("rain")] = round(
#         avg_line[data_columns.index("rain")], 3
#     )

#     cur_out.execute(sql_insert_week, avg_line)

# print("Months")
# avg_time_delta = 60 * 31
# for date in range(start_date, end_date, avg_time_delta):
#     start_time = date
#     end_time = start_time + avg_time_delta
#     query = """
#         SELECT * FROM samples
#         WHERE timestamp > {}
#         AND timestamp < {}
#         ORDER BY timestamp
#         """.format(
#         int(start_time), int(end_time)
#     )

#     cur_in = con_in.execute(query)
#     rows = cur_in.fetchall()

#     if len(rows) == 0:
#         continue
#     lines = []
#     for row in rows:
#         line = []
#         for key in data_columns:
#             line.append(getattr(row, key))
#         lines.append(line)

#         # Add "solar_voltage" for future use
#         # line.append(0.0)

#     avg_line = list(map(mean, zip(*lines)))
#     avg_line[0] = int(avg_line[0])  # Round out the timestamp to seconds
#     avg_line[1] = lines[0][1]  # Don't average the device number!
#     for idx in range(2, len(avg_line)):
#         avg_line[idx] = round(avg_line[idx], 3)

#     cur_out.execute(sql_insert_month, avg_line)

# con_out.commit()
# con_out.close()

# con_in.close()
