#!/bin/bash

export PIPENV_VENV_IN_PROJECT=true
pipenv run python -u chaac_cache.py --db /etc/chaac_cfg.yml
