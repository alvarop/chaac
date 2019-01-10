#!/bin/bash

export PIPENV_VENV_IN_PROJECT=true
export DATABASE=/home/pi/chaac.db
pipenv run gunicorn --workers 5 --bind unix:chaac_frontend.sock -m 007 src:app
