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
from chaac.chaacdb import ChaacDB

parser = argparse.ArgumentParser()

parser.add_argument("--db", required=True, help="Input dbs", nargs="*")
parser.add_argument("--out_db", help="Output db")

args = parser.parse_args()

start = time.time()

if args.out_db:
    print(f"Saving to {args.out_db}")
    out_db = ChaacDB(args.out_db)
idx = 0

for db in args.db:
    rows = []
    
    print(f"Loading {db}")
    con = None
    con = sqlite3.connect(db)

    if con is None:
        raise IOError("Unable to open sqlite database")

    cur = con.execute("PRAGMA table_info(day_samples)")
    legacy = True
    table = "day_samples"
    cols = cur.fetchall()
    if len(cols) == 0:
        cur = con.execute("PRAGMA table_info(minute_samples)")
        legacy = False
        table = "minute_samples"
        cols = cur.fetchall()
        
    
    col_names = []
    for col in cols:
        col_names.append(col[1])
    print(col_names)

    WXRecord = collections.namedtuple(f"WXRecord", col_names)

    ts = 0
    
    con.row_factory = lambda cursor,row: WXRecord(*row)
    cur = con.execute(
        """
        SELECT * FROM {} WHERE timestamp > {}
        """.format(table, ts)
    )
    rows += cur.fetchall()

    con.close() 

    print("Loaded {} rows from {}".format(len(rows), db))

    for row in rows:
        row = row._asdict()
        # print(row)
        # row["id"] = idx
        # if row["uid"] != 3130102622:
            # continue
        if row["pressure"] < 800:
            continue
        if row["pressure"] > 1100:
            continue
        if row["temperature"] < -50:
            continue
        if row["humidity"] == 0:
            continue
        if row["humidity"] > 100:
            continue
        if row["battery"] > 10:
            continue
        if row["wind_speed"] > 100:
            continue
        if row["solar_panel"] > 10:
            continue
        row = WXRecord(**row)

        if args.out_db:
            out_db.add_record(row, row.timestamp, commit=False, legacy=legacy)
        if idx % 10000 == 0:
            print("{} {}".format(idx, time.time()-start))
            start = time.time()
        idx += 1
        
if args.out_db:
    out_db.close()


# con = None
# con = sqlite3.connect(args.out_db)

# if con is None:
#     raise IOError("Unable to open sqlite database")

# cur = con.cursor()
# cur.execute(
#     "CREATE TABLE IF NOT EXISTS "
#     + "samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
#     + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
# )

# cur.execute(
#     "CREATE TABLE IF NOT EXISTS "
#     + "devices(uid INTEGER PRIMARY KEY, name TEXT, gps TEXT)"
# )

# prev_row = None
# for row in rows:
#     print(row)
#     line = []
#     for key in data_columns:
#         line.append(getattr(row, key))
#     print(line)

#     cur.execute(sql_insert, line)

# con.commit()
# con.close()
