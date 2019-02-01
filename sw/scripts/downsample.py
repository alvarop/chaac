#!/usr/bin/env python
"""
Downsample old db file
"""

import argparse
import os

import sys
import time
import json
from chaac.chaacdb import ChaacDB

parser = argparse.ArgumentParser()

parser.add_argument("--in_db", required=True, help="Sqlite db")
parser.add_argument("--out_db", help="Sqlite db")

args = parser.parse_args()

in_db = ChaacDB(args.in_db)

if args.out_db:
    out_db = ChaacDB(args.out_db)

rows = in_db.get_records("day")
in_db.close()
for row in rows:
    print(row)
    if args.out_db:
        out_db.add_record(row, row.timestamp, commit=False)

if args.out_db:
    out_db.close()
