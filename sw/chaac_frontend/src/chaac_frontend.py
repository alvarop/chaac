
from flask import Flask

import os
import collections
import sqlite3
import time
import datetime
from flask import (
    Flask,
    request,
    session,
    g,
    redirect,
    url_for,
    abort,
    render_template,
    flash,
)


app = Flask(__name__)

app.config.from_object(__name__)  # load config from this file , flaskr.py

# Load default config and override config from an environment variable
app.config.update(
    dict(DATABASE=os.path.join(app.root_path, "/home/pi/wx_out1.db"))
)

app.config.from_envvar("CHAAC_SETTINGS", silent=True)

data_columns = [
    "index",
    "timestamp",
    "temperature",
    "humidity",
    "pressure",
    "light",
    "rain",
    "wind_speed",
    "wind_dir",
]


WXRecord = collections.namedtuple("WXRecord", data_columns)


def wx_row_factory(cursor, row):
    return WXRecord(*row)


def connect_db():
    """Connects to the specific database."""
    rv = sqlite3.connect(app.config["DATABASE"])
    rv.row_factory = wx_row_factory
    return rv


def get_db():
    """Opens a new database connection if there is none yet for the
    current application context.
    """
    if not hasattr(g, "sqlite_db"):
        g.sqlite_db = connect_db()
    return g.sqlite_db


@app.teardown_appcontext
def close_db(error):
    """Closes the database again at the end of the request."""
    if hasattr(g, "sqlite_db"):
        g.sqlite_db.close()


@app.route("/")
def summary():

    # Get last sample
    db = get_db()
    query = "select * from samples order by timestamp desc limit 1"
    cur = db.execute(query)
    rows = cur.fetchall()

    samples = []
    for row in rows:
        sample = {}
        # Convert the units
        for key, val in row._asdict().items():
            if key == "timestamp":
                sample[key] = datetime.datetime.fromtimestamp(val).strftime("%Y-%m-%d %H:%M:%S")
            else:
                sample[key] = float(val)/1000.0

        samples.append(sample)

    return render_template("status.html", sample=samples[0])


@app.route("/all")
def show_all():
    db = get_db()
    cur = db.execute("select * from samples")
    rows = cur.fetchall()
    samples = []
    for row in rows:
        sample = {}
        # Convert the units
        for key, val in row._asdict().items():
            if key == "timestamp":
                sample[key] = datetime.datetime.fromtimestamp(val).strftime("%Y-%m-%d %H:%M:%S")
            else:
                sample[key] = float(val)/1000.0

        samples.append(sample)


    return render_template("show_samples.html", samples=samples)
