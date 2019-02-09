#!/bin/bash

pipenv run python -u chaac_logger.py --port /dev/ttyS0 --baud_rate 57600 --db /home/pi/chaac.db
