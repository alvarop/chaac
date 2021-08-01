import collections
import math
import numpy as np
import sqlite3
import time
from statistics import mean
from datetime import datetime, timedelta

SAMPLE_PERIOD_S = 60
HOURLY_TIME_DELTA_S = SAMPLE_PERIOD_S * 60

# timestamp and uid must be the first two
data_columns = [
    "timestamp",
    "uid",
    "temperature",
    "humidity",
    "pressure",
    "alt_temperature",
    "light",
    "battery",
    "rain",
    "wind_speed",
    "wind_gust",
    "wind_dir",
    "solar_panel",
]

# Fields we don't compute stats for
no_stat_fields = ("id", "timestamp", "uid", "wind_dir", "wind_dir_deg", "rain", "alt_temperature")

# Generate stats columns from data_columns
stat_columns = []
for field in data_columns:
    if field in no_stat_fields:
        stat_columns.append(field)
        continue

    stat_columns.append(field + "__max")
    stat_columns.append(field + "__min")
    stat_columns.append(field + "__mean")

stat_columns.append("data_period")

class KeyValueStore(collections.MutableMapping):
    """ From https://stackoverflow.com/questions/47237807/use-sqlite-as-a-keyvalue-store """

    def __init__(self, filename=None, conn=None):
        self.filename = filename

        if filename is None:
            self.conn = conn
        else:
            self.conn = sqlite3.connect(filename)

        self.conn.execute("CREATE TABLE IF NOT EXISTS kv (key text unique, value text)")
        self.c = self.conn.cursor()

    def close():
        self.conn.commit()
        if self.filename is not None:
            self.conn.close()

    def __len__(self):
        self.c.execute("SELECT COUNT(*) FROM kv")
        rows = self.c.fetchone()[0]
        return rows if rows is not None else 0

    def iterkeys(self):
        c1 = self.conn.cursor()
        for row in c1.execute("SELECT key FROM kv"):
            yield row[0]

    def itervalues(self):
        c2 = self.conn.cursor()
        for row in c2.execute("SELECT value FROM kv"):
            yield row[0]

    def iteritems(self):
        c3 = self.conn.cursor()
        for row in c3.execute("SELECT key, value FROM kv"):
            yield row[0], row[1]

    def keys(self):
        return list(self.iterkeys())

    def values(self):
        return list(self.itervalues())

    def items(self):
        return list(self.iteritems())

    def __contains__(self, key):
        self.c.execute("SELECT 1 FROM kv WHERE key = ?", (key,))
        return self.c.fetchone() is not None

    def __getitem__(self, key):
        self.c.execute("SELECT value FROM kv WHERE key = ?", (key,))
        item = self.c.fetchone()
        if item is None:
            raise KeyError(key)
        return item[0]

    def __setitem__(self, key, value):
        self.c.execute("REPLACE INTO kv (key, value) VALUES (?,?)", (key, value))
        self.conn.commit()

    def __delitem__(self, key):
        if key not in self:
            raise KeyError(key)
        self.c.execute("DELETE FROM kv WHERE key = ?", (key,))
        self.conn.commit()

    def __iter__(self):
        return self.iteritems()


class ChaacDB:
    def __init__(self, filename):
        self.conn = sqlite3.connect(filename)

        if self.conn is None:
            raise IOError("Unable to open sqlite database")

        self.cur = self.conn.cursor()

        self.WXRecord = collections.namedtuple("WXRecord", ["id"] + data_columns)
        self.WXStatRecord = collections.namedtuple(
            "WXStatRecord", ["id"] + stat_columns
        )

        self.tables = {
            "minute": "minute_samples",
            "hour": "hour_samples",
        }

        self.__init_tables()

        self.devices = {}
        self.__load_devices()

        self.config = KeyValueStore(conn=self.conn)

        self.hour_start = {}

        for device in self.devices:
            if "{}_hour_start".format(device) in self.config:
                self.hour_start[device] = int(
                    self.config["{}_hour_start".format(device)]
                )

    def close(self):
        self.__commit()
        self.conn.close()

    def __init_tables(self):
        # Table to store daily data (every minute)
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "minute_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
            + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
        )

        # Table to store hourly data 
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "hour_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
            + "{} FLOAT)".format(" FLOAT, ".join(data_columns[2:]))
        )

        # Table to store devices and locations (if available)
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "devices(uid INTEGER PRIMARY KEY, name TEXT, gps TEXT)"
        )

        # Table to store daily stats
        self.conn.execute(
            "CREATE TABLE IF NOT EXISTS "
            + "stat_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER, "
            + "{} FLOAT)".format(" FLOAT, ".join(stat_columns[2:]))
        )

        # Save the new tables
        self.__commit()

    def __load_devices(self):
        query = "SELECT * FROM devices"

        self.cur.execute(query)
        rows = self.cur.fetchall()

        for row in rows:
            self.devices[row[0]] = row[1]

    def __add_device(self, uid, name=None, gps='""'):
        self.rename_device(uid, name, gps, new=True)

        # Update downsampling time
        self.config["{}_hour_start".format(uid)] = 0
        self.hour_start[uid] = 0

    def __wx_row_factory(self, cursor, row):
        return self.WXRecord(*row)

    def rename_device(self, uid, name=None, gps=None, new=False):
        if not new and uid not in self.devices:
            raise KeyError("Unknown device!")

        if name is None:
            name = uid

        query = """
        REPLACE INTO devices (uid, name, gps) VALUES (?, ?, ?)
        """
        args = (uid, name, gps)

        # Sanitize user input
        # See https://bobby-tables.com/python
        self.cur.execute(query, args)
        self.__commit()

        self.devices[uid] = name


    def __wx_stat_row_factory(self, cursor, row):
        return self.WXStatRecord(*row)

    def get_records(
        self, table, start_date=None, end_date=None, order=None, limit=None, uid=None
    ):

        if table not in self.tables:
            raise KeyError("Invalid table!")

        self.cur.row_factory = self.__wx_row_factory

        query = "SELECT * FROM {}".format(self.tables[table])
        args = []

        options = []
        if start_date is not None:
            options.append("timestamp >= ?")
            args.append(int(start_date))

        if end_date is not None:
            options.append("timestamp < ?")
            args.append(int(end_date))

        if uid is not None:
            options.append("uid == ?")
            args.append(uid)

        if len(options) > 0:
            query += " WHERE " + " AND ".join(options)

        if order == "desc":
            query += " ORDER BY timestamp DESC"

        if limit is not None:
            query += " LIMIT ?"
            args.append(int(limit))

        self.cur.execute(query, args)

        return self.cur.fetchall()

    def __insert_line(self, line, table="minute"):
        query = "INSERT INTO {} VALUES(NULL,{})".format(
            self.tables[table], ",".join(["?"] * len(data_columns))
        )

        self.cur.execute(query, line)

    def __commit(self):
        retries = 5
        while retries > 0:
            try:
                self.conn.commit()
                break
            except sqlite3.OperationalError:
                retries -= 1
                continue        

    def __get_avg_wind_dir(self, rows):
        v_e = []
        v_n = []

        if len(rows) == 0:
            return None

        for row in rows:
            v_e.append(row.wind_speed * math.sin(row.wind_dir * math.pi/180.0))
            v_n.append(row.wind_speed * math.cos(row.wind_dir * math.pi/180.0))

        mean_dir = math.atan2(np.mean(v_e), np.mean(v_n)) * 180/math.pi
        if mean_dir < 0:
            mean_dir += 360

        return round(mean_dir,1)


    def __downsample(self, uid):
        start_time = self.hour_start[uid]
        end_time = start_time + HOURLY_TIME_DELTA_S
        query = """
            SELECT * FROM minute_samples
            WHERE timestamp >= ?
            AND timestamp < ?
            AND uid == ?
            """
        args = (int(start_time), int(end_time), uid)

        self.cur.row_factory = self.__wx_row_factory
        self.cur.execute(query, args)
        rows = self.cur.fetchall()

        if len(rows) == 0:
            # No rows to downsample!
            return

        lines = []
        for row in rows:
            line = []
            for key in data_columns:
                line.append(getattr(row, key))
            lines.append(line)

        avg_line = list(map(mean, zip(*lines)))

        # Round out the timestamp to seconds
        avg_line[data_columns.index("timestamp")] = int(
            (end_time+start_time)/2
        )

        # Don't average the device number!
        avg_line[data_columns.index("uid")] = lines[0][data_columns.index("uid")]

        # Round the rest of the items
        for idx in range(2, len(avg_line)):
            avg_line[idx] = round(avg_line[idx], 3)

        # Do fancy wind direction averaging
        avg_line[data_columns.index("wind_dir")] = self.__get_avg_wind_dir(rows)

        # Oh wait, except for rain! Add that one up instead of averaging...
        avg_line[data_columns.index("rain")] = 0
        for line in lines:
            avg_line[data_columns.index("rain")] += line[data_columns.index("rain")]
        avg_line[data_columns.index("rain")] = round(
            avg_line[data_columns.index("rain")], 3
        )

        # Save the data
        self.__insert_line(avg_line, table="hour")

    def add_record(self, record, timestamp=None, commit=True, legacy=False):
        uid = getattr(record, "uid")

        if uid not in self.devices:
            self.__add_device(uid)

        # If timestamp is none, use current time
        if timestamp is None:
            timestamp = int(time.time())

        line = [timestamp]

        for key in data_columns:
            if key != "timestamp":
                if not legacy and key == "wind_dir":
                    key = "wind_dir_deg"
                try:
                    line.append(getattr(record, key))
                except AttributeError:
                    line.append(0)

        self.__insert_line(line)
        
        """ See if current timestamp is outside of the latest
            averaging range. If so, downsample chunk and commit """

        if timestamp > (self.hour_start[uid] + HOURLY_TIME_DELTA_S):
            self.__downsample(uid)
            self.hour_start[uid] = timestamp - timestamp % (60 * 60)
            self.config["{}_hour_start".format(uid)] = timestamp - timestamp % (60 * 60)

        if commit:
            self.__commit()

        # Compute stats for yesterday (if needed)
        start_time = datetime.fromtimestamp(timestamp).replace(
            minute=0, second=0, hour=0
        ) - timedelta(days=1)
        end_time = datetime.fromtimestamp(timestamp).replace(minute=0, second=0, hour=0)
        self.__compute_stats(
            start_time.timestamp(),
            end_time.timestamp(),
            uid=getattr(record, "uid"),
            commit=commit,
        )

    def get_stats(
        self, start_date=None, end_date=None, order=None, limit=None, uid=None
    ):

        self.cur.row_factory = self.__wx_stat_row_factory

        if uid == None:
            raise ValueError("uid required for stats")

        query = "SELECT * FROM stat_samples"
        args = []
        options = []

        if start_date is not None:
            options.append("timestamp >= ?")
            args.append(int(start_date))

        if end_date is not None:
            options.append("timestamp < ?")
            args.append(int(end_date))

        if uid is not None:
            options.append("uid == ?")
            args.append(uid)

        if len(options) > 0:
            query += " WHERE " + " AND ".join(options)

        if order == "desc":
            query += " ORDER BY timestamp DESC"

        if limit is not None:
            query += " LIMIT ?"
            args.append(int(limit))

        self.cur.execute(query, args)

        return self.cur.fetchall()

    def __compute_stats(self, start_time, end_time, uid, commit=True, delete_old_samples=True):

        stat = self.get_stats(start_time, end_time, uid=uid)

        # No need to compute stats if they're already present
        if len(stat) == 1:
            return stat[0]

        rows = self.get_records(
            "minute", start_date=start_time, end_date=end_time, uid=uid
        )

        if len(rows) == 0:
            return None

        # Use numpy for faster everything
        row_array = np.array(rows)

        # Compute max/min/mean for all values
        day_max = self.WXRecord(*np.amax(row_array, axis=0))
        day_min = self.WXRecord(*np.amin(row_array, axis=0))
        day_mean = self.WXRecord(*np.around(np.mean(row_array, axis=0), decimals=3))

        # Compute rain totals
        rain_total = 0
        for row in rows:
            rain_total += row.rain

        # Create dict with all stats
        day_stats = {
            "id": None,
            "timestamp": start_time,
            "uid": uid,
            "wind_dir": None,
            "rain": round(rain_total,2),
            "data_period": int(end_time - start_time),
        }

        for field in self.WXRecord._fields:
            if field in no_stat_fields:
                continue

            day_stats[field + "__max"] = getattr(day_max, field)
            day_stats[field + "__min"] = getattr(day_min, field)
            day_stats[field + "__mean"] = getattr(day_mean, field)

        line = []
        for key in self.WXStatRecord._fields:
            line.append(day_stats.get(key))

        query = "INSERT INTO stat_samples VALUES({})".format(
            ",".join(["?"] * len(self.WXStatRecord._fields))
        )

        self.cur.execute(query, line)

        if commit:
            self.__commit()

        # Delete samples older than a day
        if delete_old_samples:
            query = "DELETE FROM minute_samples WHERE TIMESTAMP < ? AND UID == ?"
            self.cur.execute(query, [start_time - (60 * 60 * 24 * 2), uid])

            if commit:
                self.__commit()
