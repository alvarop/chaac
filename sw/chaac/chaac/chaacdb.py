import collections
import sqlite3
import time
from statistics import mean

SAMPLE_PERIOD_S = 60
WEEK_TIME_DELTA_S = SAMPLE_PERIOD_S * 7
MONTH_TIME_DELTA_S = SAMPLE_PERIOD_S * 31

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

        self.WXRecord = collections.namedtuple("WXRecord", ["id"] + data_columns)

        self.tables = {
            "day": "day_samples",
            "week": "week_samples",
            "month": "month_samples",
        }

        self.__init_tables()

        self.config = KeyValueStore(conn=self.conn)

        if "week_sample_start" not in self.config:
            self.config["week_sample_start"] = 0

        if "month_sample_start" not in self.config:
            self.config["month_sample_start"] = 0

        # Use these to keep track of averaged samples
        self.week_sample_start = int(self.config["week_sample_start"])
        self.month_sample_start = int(self.config["month_sample_start"])

        self.cur = self.conn.cursor()

    def close(self):
        self.__commit()
        self.conn.close()

    def __init_tables(self):
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

        # Save the new tables
        self.__commit()

    def __wx_row_factory(self, cursor, row):
        return self.WXRecord(*row)

    def get_records(
        self, table, start_date=None, end_date=None, order=None, limit=None
    ):
        if table not in self.tables:
            raise KeyError("Invalid table!")

        self.cur.row_factory = self.__wx_row_factory

        query = "SELECT * FROM {}".format(self.tables[table])

        if start_date is not None:
            query += " WHERE timestamp >= {}".format(int(start_date))

        # TODO - handle end data with no start date
        if end_date is not None:
            query += " AND timestamp < {}".format(int(end_date))

        if order == "desc":
            query += " ORDER BY timestamp DESC"

        if limit is not None:
            query += " LIMIT {}".format(int(limit))

        self.cur.execute(query)

        return self.cur.fetchall()

    def __insert_line(self, line, table="day"):
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

    def __downsample_check(self, timestamp):
        """ See if current timestamp is outside of the latest
            averaging range. If so, downsample chunk and commit """

        if timestamp > (self.week_sample_start + WEEK_TIME_DELTA_S):
            self.__downsample("week")
            self.week_sample_start = timestamp
            self.config["week_sample_start"] = timestamp

        if timestamp > (self.month_sample_start + MONTH_TIME_DELTA_S):
            self.__downsample("month")
            self.month_sample_start = timestamp
            self.config["month_sample_start"] = timestamp

    def __downsample(self, table):
        if table == "week":
            start_time = self.week_sample_start
            end_time = start_time + WEEK_TIME_DELTA_S
        elif table == "month":
            start_time = self.month_sample_start
            end_time = start_time + MONTH_TIME_DELTA_S
        else:
            raise ValueError("Invalid table!")

        query = """
            SELECT * FROM day_samples
            WHERE timestamp >= {}
            AND timestamp < {}
            """.format(
            int(start_time), int(end_time)
        )

        self.cur.row_factory = self.__wx_row_factory
        self.cur.execute(query)
        rows = self.cur.fetchall()

        if len(rows) == 0:
            # TODO - get rid of these debug prints
            print("No rows to downsample :(")
            # No rows to downsample!
            return
        else:
            # TODO - get rid of these debug prints
            print(
                "Downsampling {} rows for {}. ({}, {})".format(
                    len(rows), table, start_time, end_time
                )
            )

        lines = []
        for row in rows:
            line = []
            for key in data_columns:
                line.append(getattr(row, key))
            lines.append(line)

        avg_line = list(map(mean, zip(*lines)))

        # Round out the timestamp to seconds
        avg_line[data_columns.index("timestamp")] = int(
            avg_line[data_columns.index("timestamp")]
        )

        # Don't average the device number!
        avg_line[data_columns.index("uid")] = lines[0][data_columns.index("uid")]

        # Round the rest of the items
        for idx in range(2, len(avg_line)):
            avg_line[idx] = round(avg_line[idx], 3)

        # Oh wait, except for rain! Add that one up instead of averaging...
        avg_line[data_columns.index("rain")] = 0
        for line in lines:
            avg_line[data_columns.index("rain")] += line[data_columns.index("rain")]
        avg_line[data_columns.index("rain")] = round(
            avg_line[data_columns.index("rain")], 3
        )

        # Save the data
        self.__insert_line(avg_line, table=table)

    def add_record(self, record, timestamp=None, commit=True):
        # If timestamp is none, use current time
        if timestamp is None:
            timestamp = int(time.time())

        line = [timestamp]

        for key in data_columns:
            if key != "timestamp":
                line.append(getattr(record, key))

        self.__insert_line(line)

        self.__downsample_check(timestamp)

        if commit:
            self.__commit()
