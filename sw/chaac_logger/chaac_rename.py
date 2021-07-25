#!/usr/bin/env python
"""
Decode incoming packets from weather station and save to db
"""

import argparse
from chaac.chaacdb import ChaacDB

parser = argparse.ArgumentParser()

parser.add_argument("db", help="Sqlite db file")
parser.add_argument("uid", help="device uid")
parser.add_argument("name", help="new device name")


args = parser.parse_args()

db = ChaacDB(args.db)

db.rename_device(int(args.uid), name=args.name)
