#!/bin/bash

export DATABASE=/home/pi/chaac.db
gunicorn --workers 5 --bind unix:chaac_frontend.sock -m 007 src:app
