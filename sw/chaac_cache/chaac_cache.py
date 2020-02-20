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
    "San Mateo": "10.1.1.10",
    "Merida": "10.1.1.11",
    "chaac-dev": "10.1.1.20",
}

default_config = {
    "hosts": default_hosts,
    "cache_dir": "/tmp/chaac_cache",
    "refresh_rate": 50,
    "request_timeout": 10,
}

parser = argparse.ArgumentParser()
parser.add_argument("--config", required=True, help="chaac config file")
args = parser.parse_args()

# Create config if not there
if not os.path.exists(args.config):
    print("Creating default config in " + args.config)
    with open(args.config, "w") as outfile:
        yaml.dump(default_config, outfile)

with open(args.config, "r") as config_file:
    config = yaml.safe_load(config_file)

if not os.path.exists(config["cache_dir"]):
    print("Creating cache dir " + config["cache_dir"])
    os.makedirs(config["cache_dir"])
# print(yaml.dump(config))


def json_from_url(url):
    try:
        json_request = requests.get(url, timeout=config["request_timeout"])
        if json_request.status_code != 200:
            print(url, "Request error ({})".format(json_request.status_code))
            return None

        return json_request.text
    except requests.exceptions.ReadTimeout:
        print(url, "Read Timeout")
    except requests.exceptions.ConnectionError:
        print(url, "Connection Error")
        return None


while 1:
    for name, ip in config["hosts"].items():
        json = json_from_url("http://" + ip + "/latest")

        cache_filename = "{}/{}.yml".format(config["cache_dir"], name)

        if json is not None:
            cache = {
                "timestamp": time.time(),
                "json": base64.b64encode(json.encode("utf-8")).decode("utf-8"),
            }

            with open(cache_filename, "w") as cache_file:
                yaml.dump(cache, cache_file)
        else:
            if os.path.exists(cache_filename):
                print("Removing cache file " + cache_filename)
                os.remove(cache_filename)

    time.sleep(config["refresh_rate"])
