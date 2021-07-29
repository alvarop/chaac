#!/usr/bin/env python
"""
Ping chaac servers and cache their data
"""

import argparse
import time
import base64
import requests
import yaml
import os

default_hosts = {
    "Merida": "localhost:8000/Merida",
    "Sunbright": "localhost:8000/Sunbright",
}

default_config = {
    "hosts": default_hosts,
    "refresh_rate": 10,
    "request_timeout": 10,
    "verbose": False
}

parser = argparse.ArgumentParser()
parser.add_argument("--config", help="chaac config file")
args = parser.parse_args()

if args.config is not None:
    config_path = args.config
else:
    config_path = os.environ.get('CONFIG')

# Create config if not there
if not os.path.exists(config_path):
    config = default_config
else:
    with open(config_path, "r") as config_file:
        config = yaml.safe_load(config_file)

def get_url(url, verbose=False):
    if verbose:
        print(f"Getting {url}")
    try:
        url_request = requests.get(url, timeout=config["request_timeout"])
        if url_request.status_code != 200:
            print(url, "Request error ({})".format(url_request.status_code))
            return None

        return url_request.text
    except requests.exceptions.ReadTimeout:
        if verbose:
            print("Timeout")
        return None
    except requests.exceptions.ConnectionError:
        if verbose:
            print("Connection Error")
        return None

print("Chaac cache config:", config)

while 1:
    for name, ip in config["hosts"].items():
        get_url("http://" + ip + "/", config["verbose"])
        get_url("http://" + ip + "/latest", config["verbose"])

    time.sleep(config["refresh_rate"])
