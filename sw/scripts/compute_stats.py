#!/usr/bin/env python
"""
Compute statistics for chaac db
"""

import argparse
import os
import numpy as np
import sys
import time
import json
from chaac.chaacdb import ChaacDB
from datetime import datetime, timedelta

parser = argparse.ArgumentParser()
parser.add_argument("db", help="Sqlite db")
args = parser.parse_args()

chaac = ChaacDB(args.db)

for uid in chaac.devices:

    try:
        first_sample = chaac.get_records("day", order="asc", limit=1, uid=uid)[0]
        last_sample = chaac.get_records("day", order="desc", limit=1, uid=uid)[0]
    except IndexError:
        continue

    start_day = datetime.fromtimestamp(first_sample.timestamp).replace(
        minute=0, second=0, hour=0
    )
    end_day = datetime.fromtimestamp(last_sample.timestamp).replace(
        minute=0, second=0, hour=0
    ) + timedelta(days=1)

    print(
        "Computing stats for "
        + chaac.devices[uid]
        + " from "
        + start_day.strftime("%Y-%m-%d %H:%M:%S")
        + " to "
        + end_day.strftime("%Y-%m-%d %H:%M:%S")
    )

    current_day = start_day
    while current_day < end_day:
        next_day = current_day + timedelta(days=1)

        day_stats = chaac._ChaacDB__compute_stats(
            current_day.timestamp(), next_day.timestamp(), uid=uid, commit=True
        )

        print(current_day)
        print(day_stats)

        current_day = next_day

chaac.close()

