#!/bin/bash

export PIPENV_VENV_IN_PROJECT=true
pipenv run gunicorn --workers 5 --bind unix:chaac_server.sock -m 007 src:app
