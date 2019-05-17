
import os
import time
import socket
import requests
from datetime import datetime, timedelta
from flask import Flask, request, g, render_template, jsonify, Response

app = Flask(__name__)

app.config.from_object(__name__)  # load config from this file , flaskr.py

# Load default config and override config from an environment variable
# app.config.update(dict(DATABASE=os.getenv("DATABASE")))

hosts = {"San Mateo": "10.1.1.10", "Merida": "10.1.1.11"}


def json_error(message):
    error["error"] = message
    return jsonify(error)


def json_from_url(url):
    try:
        json_request = requests.get(url)
        if json_request.status_code != 200:
            return json_error("Request error ({})".format(json_request.status_code))

        return Response(json_request.text, mimetype='application/json')
    except ConnectionError:
        return json_error("Error connecting to host")


@app.route("/<host>/latest")
def latest_json(host):
    if host not in hosts:
        return json_error("Invalid host")

    return json_from_url("http://" + hosts[host] + "/latest")


@app.route("/<host>/json/day")
def json_day_str(host):
    if host not in hosts:
        return json_error("Invalid host")

    return json_from_url("http://" + hosts[host] + "/json/day")


@app.route("/<host>/json/week")
def json_week_str(host):
    if host not in hosts:
        return json_error("Invalid host")

    return json_from_url("http://" + hosts[host] + "/json/week")


@app.route("/<host>/json/month")
def json_month_str(host):
    if host not in hosts:
        return json_error("Invalid host")

    return json_from_url("http://" + hosts[host] + "/json/month")


@app.route("/<host>")
def summary(host):
    if host not in hosts:
        return render_template("error.html", error_string="Error: Invalid Host.")

    return render_template("status.html", host=host)


@app.route("/<host>/plots")
def plots(host):
    if host not in hosts:
        return render_template("error.html", error_string="Error: Invalid Host.")

    return render_template("plots.html", host=host)

@app.route("/")
def list():
    return render_template("hosts.html", hosts=hosts.keys())