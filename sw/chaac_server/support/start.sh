#!/bin/bash

export PIPENV_VENV_IN_PROJECT=true
export CONFIG=/etc/chaac/chaac_cfg.yml
pipenv run gunicorn --workers 5 --bind unix:chaac_server.sock -m 007 src:app
