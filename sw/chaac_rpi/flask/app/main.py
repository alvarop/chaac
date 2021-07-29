import os
import time
import socket
import zipfile
from datetime import datetime, timedelta
from flask import Flask, request, g, render_template, jsonify, redirect, Response
from chaac.chaacdb import ChaacDB

app = Flask(__name__)

app.config.from_object(__name__)  # load config from this file , flaskr.py

# Load default config and override config from an environment variable
app.config.update(dict(DATABASE=os.getenv("DATABASE")))

def get_pretty_hostname():
    hostname = socket.gethostname()

    if "chaac-" in hostname:
        hostname = " ".join(hostname.split('-')[1:]).title()
    else:
        hostname = " ".join(hostname.split('-')).title()

    return hostname

hostname = get_pretty_hostname()

default_uid = None


def get_db():
    """Opens a new database connection if there is none yet for the
    current application context.
    """
    if not hasattr(g, "sqlite_db"):
        g.sqlite_db = ChaacDB(app.config["DATABASE"])
    return g.sqlite_db


@app.teardown_appcontext
def close_db(error):
    """Closes the database again at the end of the request."""
    if hasattr(g, "sqlite_db"):
        g.sqlite_db.close()


def get_latest_sample(uid):
    """ Get latest weather data (and past day's rainfall) """

    # Get last sample
    db = get_db()

     # Past day
    now = datetime.fromtimestamp(int(time.time()))
    end_time = start_time = time.mktime(now.timetuple())

    # Start at midnight today
    start_time = time.mktime(now.replace(hour=0, minute=0, second=0).timetuple())


    rows = db.get_records("minute", order="desc", uid=uid)

    sample = {}
    # Convert the units
    for key, val in rows[0]._asdict().items():
        if key == "timestamp":
            sample[key] = datetime.fromtimestamp(val).strftime("%Y-%m-%d %H:%M:%S")
            sample["ts"] = val
        elif val == None:
            sample[key] = 0
        else:
            sample[key] = round(float(val), 2)


    rain_total = 0
    for row in rows:
        rain_total += row.rain

    sample["rain"] = round(rain_total, 2)

    return sample


@app.route("/latest")
def latest_json():
    db = get_db()
    data = {"hostname": hostname, "devices":{}}
    for device, name in db.devices.items():
        sample = get_latest_sample(device)

        # Don't show devices with data older than 24 hours
        if (time.time() - sample["ts"]) < 60 * 60 * 24:
            data["devices"][device] = get_latest_sample(device)
            data["devices"][device]["name"] = name

    return jsonify(data)


@app.route("/")
def summary():
    return render_template("status.html", hostname=hostname)


rain_mod = {"day": (60 * 60), "week": (60 * 60 * 24), "month": (60 * 60 * 24)}


def get_start_bin(end_date, table):
    """ Figure out what time it is now to start the bar chart
        The numbers depend on whether it's a day/week/month plot
    """

    # Start one day (% rain_mod) after today
    end_date += rain_mod[table]
    if table == "day":
        return datetime.fromtimestamp(end_date).timetuple().tm_hour
    elif table == "week":
        return datetime.fromtimestamp(end_date).timetuple().tm_wday
    elif table == "month":
        return datetime.fromtimestamp(end_date).timetuple().tm_mday
    else:
        return None


days = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]


def get_rain_label(idx, table):
    """ Get nice labels for the bar chart. Unfortunately, plotly
    keeps re-sorting all the numbers so we have to add strings
    around them to keep it in the correct order. """

    if table == "day":
        return "(" + str(idx) + ")"
    elif table == "week":
        return days[idx]
    elif table == "month":
        # TODO: Deal with days=0 etc
        return "(" + str(idx) + ")"
    else:
        return None


def get_data_dict(uid, start_date, end_date, table="day"):
    """ Get weather data for the specified weather period """
    db = get_db()

    if table == "day":
        real_table = "minute"
    else:
        real_table = "hour"
    rows = db.get_records(real_table, start_date=start_date, end_date=end_date, uid=uid)
    if len(rows) == 0:
        return None

    plot = {}
    col_names = rows[0]._asdict().keys()

    for name in col_names:
        plot[name] = []

    # Rain doesn't have the same timestamp as the rest of the data
    plot["rain_time"] = []

    # Create lists with each data type and make timestamp pretty
    for row in rows:
        for name in col_names:
            if name == "timestamp":
                plot[name].append(
                    datetime.fromtimestamp(getattr(row, name)).strftime(
                        "%Y-%m-%d %H:%M:%S"
                    )
                )
            elif name == "uid" or name == "id" or name == "rain":
                continue
            else:
                if getattr(row, name) is None:
                    plot[name].append(0)
                else:
                    plot[name].append(round(getattr(row, name), 3))

    # Bin data into the appropriate size for histograms
    idx = get_start_bin(int(end_date - 1), table)
    bins = range(int(start_date), int(end_date), rain_mod[table])

    # Loop through each rain bin
    for rain_bin in bins:
        plot["rain_time"].append(get_rain_label(idx, table))
        rain = 0
        # Loop through each rain sample
        for row in rows:
            if row.rain == 0:
                continue
            # Check if the sample falls into our bin
            if row.timestamp >= rain_bin and row.timestamp < (rain_bin + rain_mod[table]):
                rain += row.rain

        plot["rain"].append(rain)

        # Wrap around depending on the number of bins (since we don't always start at 0)
        idx = (idx + 1) % len(bins)

    return plot

def join_data(prev_data, new_data):
    for key in prev_data.keys():
        if prev_data["timestamp"][-1] < new_data["timestamp"][0]:
            prev_data[key] += new_data[key]
        else:
            prev_data[key] = new_data[key] + prev_data[key]
    return prev_data

def get_stats(uid, start_date, end_date):
    """ Get weather data for the specified weather period """

    db = get_db()

    rows = db.get_stats(start_date=start_date, end_date=end_date, uid=uid)
    if len(rows) == 0:
        return
    plot = {}

    col_names = rows[0]._asdict().keys()

    plot["stat_fields"] = []
    for name in col_names:
        split_name = name.rsplit("__")
        if len(split_name) > 1:
            if split_name[0] not in plot:
                plot[split_name[0]] = {}
                plot["stat_fields"].append(split_name[0])
            plot[split_name[0]][split_name[1]] = []
        else:
            plot[name] = []

    ignored_fields = ["uid", "id", "data_period", "temperature_in", "wind_dir"]

    # Create lists with each data type and make timestamp pretty
    for row in rows:
        for name in col_names:
            if name == "timestamp":
                plot[name].append(
                    datetime.fromtimestamp(getattr(row, name)).strftime("%Y-%m-%d")
                )
            elif name in ignored_fields:
                continue
            else:
                split_name = name.rsplit("__")
                if len(split_name) > 1:
                    if getattr(row, name) is None:
                        plot[split_name[0]][split_name[1]].append(0)
                    else:
                        plot[split_name[0]][split_name[1]].append(
                            round(getattr(row, name), 3)
                        )
                else:
                    if getattr(row, name) is None:
                        plot[name].append(0)
                    else:
                        plot[name].append(round(getattr(row, name), 3))

    return plot


@app.route("/json/stats/year")
def json_stats_year_str():
    db = get_db()

    # time.time() is utc time, but now is a "naive"
    # datetime object in current timezone
    now = datetime.fromtimestamp(int(time.time()))

    # Start this year before the next full hour
    start_time = time.mktime(
        (now.replace(minute=0, second=0, hour=0, day=1, month=1)).timetuple()
    )

    end_time = time.mktime(now.timetuple())

    stats = {"hostname": hostname}
    stats["start_date"] = datetime.fromtimestamp(start_time).strftime("%Y-%m-%d")
    stats["end_date"] = datetime.fromtimestamp(end_time).strftime("%Y-%m-%d")
    stats["devices"] = {}
    for device, name in db.devices.items():
        uid_stats = get_stats(device, start_time, end_time)
        if uid_stats is not None:
            stats["devices"][name] = uid_stats

    return jsonify(stats)


@app.route("/json/day")
def json_day_str():
    # time.time() is utc time, but now is a "naive"
    # datetime object in current timezone
    now = datetime.fromtimestamp(int(time.time()))

    # Start 24 hours before the next full hour
    start_time = time.mktime(
        (
            now.replace(minute=0, second=0) + timedelta(hours=1) - timedelta(days=1)
        ).timetuple()
    )

    end_time = time.mktime(now.timetuple())
    db = get_db()

    data = {"hostname": hostname}
    data["start_date"] = datetime.fromtimestamp(start_time).strftime(
        "%Y-%m-%d %H:%M:%S"
    )
    data["end_date"] = datetime.fromtimestamp(end_time).strftime("%Y-%m-%d %H:%M:%S")
    data["data"] = {}

    for device, name in db.devices.items():
        data_dict = get_data_dict(device, start_time, end_time, "day")
        if data_dict is not None:
            if name in data["data"]:
                data["data"][name] = join_data(data["data"][name], data_dict)
            else:
                data["data"][name] = data_dict
    return jsonify(data)


@app.route("/json/week")
def json_week_str():
    # time.time() is utc time, but now is a "naive"
    # datetime object in current timezone
    now = datetime.fromtimestamp(int(time.time()))

    # Round to the full day, start 7 days ago
    start_time = time.mktime(
        (
            now.replace(hour=0, minute=0, second=0)
            + timedelta(days=1)
            - timedelta(weeks=1)
        ).timetuple()
    )

    end_time = time.mktime(now.timetuple())
    db = get_db()
    data = {"hostname": hostname}
    data["start_date"] = datetime.fromtimestamp(start_time).strftime(
        "%Y-%m-%d %H:%M:%S"
    )
    data["end_date"] = datetime.fromtimestamp(end_time).strftime("%Y-%m-%d %H:%M:%S")
    data["data"] = {}

    for device, name in db.devices.items():
        data_dict = get_data_dict(device, start_time, end_time, "week")
        if data_dict is not None:
            if name in data["data"]:
                data["data"][name] = join_data(data["data"][name], data_dict)
            else:
                data["data"][name] = data_dict
    return jsonify(data)


@app.route("/json/month")
def json_month_str():
    # time.time() is utc time, but now is a "naive"
    # datetime object in current timezone
    now = datetime.fromtimestamp(int(time.time()))

    # TODO - round to the month?
    # Round to the full day, start 31 days ago
    start_time = time.mktime(
        (
            now.replace(hour=0, minute=0, second=0)
            + timedelta(days=1)
            - timedelta(days=31)
        ).timetuple()
    )

    end_time = time.mktime(now.timetuple())

    db = get_db()
    data = {"hostname": hostname}
    data["start_date"] = datetime.fromtimestamp(start_time).strftime(
        "%Y-%m-%d %H:%M:%S"
    )
    data["end_date"] = datetime.fromtimestamp(end_time).strftime("%Y-%m-%d %H:%M:%S")
    data["data"] = {}

    for device, name in db.devices.items():
        data_dict = get_data_dict(device, start_time, end_time, "month")
        if data_dict is not None:
            if name in data["data"]:
                data["data"][name] = join_data(data["data"][name], data_dict)
            else:
                data["data"][name] = data_dict
    return jsonify(data)


@app.route("/plots")
def plots():
    return render_template("plots.html", hostname=hostname)


@app.route("/stats")
def stats():
    return render_template("stats.html", hostname=hostname)

# Don't add hostname to redirect
# See https://stackoverflow.com/questions/30006740/how-can-i-tell-flask-not-to-add-host-scheme-info-to-my-redirect
class NoHostnameResponse(Response):
    autocorrect_location_header = False

@app.route("/zipdb")
def download_zip_db():
    dbname = "chaac.db.zip"
    with zipfile.ZipFile(
        f"/tmp/files/{dbname}", "w", compression=zipfile.ZIP_DEFLATED
    ) as dbzip:
        print("Zipping ", os.getenv("DATABASE"))
        dbzip.write(os.getenv("DATABASE"))
    return redirect(f"files/{dbname}", Response=NoHostnameResponse)
