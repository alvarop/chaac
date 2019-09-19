#!/bin/bash

echo "Updating files"
sudo rsync --chown=www-data:www-data -a `git rev-parse --show-toplevel`/sw/chaac_cache /var/flaskapp/

cd `git rev-parse --show-toplevel`/sw/chaac_cache

echo "Updating supervisor config setup"
sudo rm -f /etc/supervisor/conf.d/chaac_cache.conf
sudo cp support/chaac_cache.conf /etc/supervisor/conf.d/chaac_cache.conf

cd -

echo "restart supervisor"
sudo systemctl restart supervisor
