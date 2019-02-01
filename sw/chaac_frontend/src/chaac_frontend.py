
import os
import time
from datetime import datetime
from flask import Flask, request, g, render_template, jsonify
from chaac.chaacdb import ChaacDB

app = Flask(__name__)

app.config.from_object(__name__)  # load config from this file , flaskr.py

# Load default config and override config from an environment variable
app.config.update(dict(DATABASE=os.getenv("DATABASE")))


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


@app.route("/")
def summary():

    # Get last sample
    db = get_db()
    rows = db.get_records("day", order="desc", limit=1)

    sample = {}
    # Convert the units
    for key, val in rows[0]._asdict().items():
        if key == "timestamp":
            sample[key] = datetime.fromtimestamp(val).strftime("%Y-%m-%d %H:%M:%S")
        else:
            sample[key] = round(float(val), 2)

    return render_template("status.html", sample=sample)


def get_json_str(start_date, end_date):

    td = int(end_date) - int(start_date)
    if td <= 60 * 60 * 24:
        table = "day"
    elif td <= 60 * 60 * 24 * 7:
        table = "week"
    else:  # if td <= 60 * 24 * 31:
        table = "month"

    db = get_db()

    rows = db.get_records(table, start_date=start_date, end_date=end_date)

    plot = {}

    plot["start_date"] = datetime.fromtimestamp(start_date).strftime(
        "%Y-%m-%d %H:%M:%S"
    )
    plot["end_date"] = datetime.fromtimestamp(end_date).strftime("%Y-%m-%d %H:%M:%S")

    col_names = rows[0]._asdict().keys()

    for name in col_names:
        plot[name] = []

    for row in rows:
        for name in col_names:
            if name == "timestamp":
                plot[name].append(
                    datetime.fromtimestamp(getattr(row, name)).strftime(
                        "%Y-%m-%d %H:%M:%S"
                    )
                )
            elif name == "uid" or name == "id":
                continue
            else:
                plot[name].append(round(getattr(row, name), 3))

    return jsonify(plot)


@app.route("/json/day")
def json_day_str():
    return get_json_str(time.time() - 60 * 60 * 24, time.time())


@app.route("/json/week")
def json_week_str():
    return get_json_str(time.time() - 60 * 60 * 24 * 7, time.time())


@app.route("/json/month")
def json_month_str():
    return get_json_str(time.time() - 60 * 60 * 24 * 31, time.time())


@app.route("/plots")
def test():
    return render_template("plots.html")
