#!/bin/bash

pipenv run python chaac_logger.py --port /dev/ttyS0 --baud_rate 115200 --db /home/pi/chaac.db
