#!/bin/bash

export CONFIG=/etc/chaac/chaac_cfg.yml
gunicorn --workers 5 --bind unix:chaac_server.sock -m 007 src:app
