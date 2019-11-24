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


def cached_result(host):
    if host not in config["hosts"]:
        print("Invalid host " + host)
        return None

    cache_filename = "{}/{}.yml".format(config["cache_dir"], host)
    if os.path.exists(cache_filename):
        with open(cache_filename, "r") as cache_file:
            cache = yaml.safe_load(cache_file)
            # if (time.time() - cache["timestamp"]) < expiration_s:
            return base64.b64decode(cache["json"]).decode("utf-8")

    return None


def json_from_url(url):
    try:
        json_request = requests.get(url)
        if json_request.status_code != 200:
            return json_error("Request error ({})".format(json_request.status_code))

        return Response(json_request.text, mimetype="application/json")
    except ConnectionError:
        return json_error("Error connecting to host")


@app.route("/<host>")
def summary(host):
    if host not in config["hosts"]:
        return render_template("error.html", error_string="Error: Invalid Host.")

    return render_template("status.html", host=host)


@app.route("/<host>/plots")
def plots(host):
    if host not in config["hosts"]:
        return render_template("error.html", error_string="Error: Invalid Host.")

    return render_template("plots.html", host=host)

@app.route("/<host>/stats")
def stats(host):
    if host not in config["hosts"]:
        return render_template("error.html", error_string="Error: Invalid Host.")

    return render_template("stats.html", host=host)

# Forward every other request down to host
@app.route("/<host>/<path:path>")
def latest_json(host, path):
    if host not in config["hosts"]:
        return json_error("Invalid host")

    if path == "latest":
        cached_json = cached_result(host)
    else:
        cached_json = None   

    if cached_json:
        return cached_json
    else:
        return json_from_url("http://" + config["hosts"][host] + "/" + path)


@app.route("/")
def list():
    return render_template("hosts.html", hosts=config["hosts"].keys())
