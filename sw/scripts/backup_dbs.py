#!/usr/bin/env python
"""
Backup chaac db's for each host.
Example:
pipenv run python backup_dbs.py --config ../path/to/chaac_cfg.yml /tmp/backup

"""

import argparse
import os
import yaml
from subprocess import Popen, PIPE, STDOUT

parser = argparse.ArgumentParser()

parser.add_argument(
    "--config", default="/etc/chaac/chaac_cfg.yml", help="Config file location"
)
parser.add_argument("destination", help="Backup directory")

parser.add_argument("-v", action="store_true", help="Verbose output")

args = parser.parse_args()

config = None
with open(args.config, "r") as config_file:
    config = yaml.safe_load(config_file)

if config is None:
    raise Exception("Unable to load config file")

if not os.path.exists(args.destination):
    print("Creating " + args.destination)
    os.mkdir(args.destination)

for host in config["hosts"]:
    # TODO - Check for valid hostname
    dest_dir = args.destination + "/" + host + "/"

    if not os.path.exists(dest_dir):
        print("Creating " + dest_dir)
        os.mkdir(dest_dir)

    cmd = [
        "rsync",
        "--timeout=30",
        "-avz",
        "pi@{}:/home/pi/chaac.db".format(config["hosts"][host]),
        dest_dir,
    ]

    print("Backing up {} from {}".format(host, config["hosts"][host]))

    if args.v:
        print(" ".join(cmd))

    out = Popen(cmd, stderr=STDOUT, stdout=PIPE)
    output = out.communicate()[0]

    if args.v:
        print(output.decode("utf-8"))

    if out.returncode == 0:
        print("Success!")
    else:
        print("Error getting file")
