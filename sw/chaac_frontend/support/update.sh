#!/bin/bash

echo "Updating files"
sudo rsync --chown=www-data:www-data -a `git rev-parse --show-toplevel`/sw/chaac_frontend /var/flaskapp/

echo "restart nginx"
sudo systemctl restart nginx

echo "restart supervisor"
sudo systemctl restart supervisor
