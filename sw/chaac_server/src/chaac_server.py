
import os
import time
import socket
import requests
import base64
import yaml
from datetime import datetime, timedelta
from flask import Flask, request, g, render_template, jsonify, Response

app = Flask(__name__)

app.config.from_object(__name__)  # load config from this file , flaskr.py

config = None
with open(os.getenv("CONFIG"), "r") as config_file:
    config = yaml.safe_load(config_file)

if config is None:
    raise Exception("Unable to load config file")

def json_error(message):
    error = {}
    error["error"] = message
    return jsonify(error)

def cached_result(host, expiration_s = 120):
    if host not in config['hosts']:
        print("Invalid host " + host)
        return None

    cache_filename = "{}/{}.yml".format(config["cache_dir"], host)
    with open(cache_filename, 'r') as cache_file:
        cache = yaml.safe_load(cache_file)
        if (time.time() - cache['timestamp']) < expiration_s:
             return base64.b64decode(cache['json']).decode('utf-8')

    return None


def json_from_url(url):
    try:
        json_request = requests.get(url)
        if json_request.status_code != 200:
            return json_error("Request error ({})".format(json_request.status_code))

        return Response(json_request.text, mimetype="application/json")
    except ConnectionError:
        return json_error("Error connecting to host")


@app.route("/<host>/latest")
def latest_json(host):
    if host not in config['hosts']:
        return json_error("Invalid host")

    cached_json = cached_result(host)
    if cached_json:
        return cached_json    
    else:
        return json_from_url("http://" + config['hosts'][host] + "/latest")


@app.route("/<host>/json/day")
def json_day_str(host):
    if host not in config['hosts']:
        return json_error("Invalid host")

    return json_from_url("http://" + config['hosts'][host] + "/json/day")


@app.route("/<host>/json/week")
def json_week_str(host):
    if host not in config['hosts']:
        return json_error("Invalid host")

    return json_from_url("http://" + config['hosts'][host] + "/json/week")


@app.route("/<host>/json/month")
def json_month_str(host):
    if host not in config['hosts']:
        return json_error("Invalid host")

    return json_from_url("http://" + config['hosts'][host] + "/json/month")


@app.route("/<host>")
def summary(host):
    if host not in config['hosts']:
        return render_template("error.html", error_string="Error: Invalid Host.")

    return render_template("status.html", host=host)


@app.route("/<host>/plots")
def plots(host):
    if host not in config['hosts']:
        return render_template("error.html", error_string="Error: Invalid Host.")

    return render_template("plots.html", host=host)


@app.route("/")
def list():
    return render_template("hosts.html", hosts=config['hosts'].keys())
