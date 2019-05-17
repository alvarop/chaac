#!/bin/bash

echo "Updating files"
sudo rsync --chown=www-data:www-data -a `git rev-parse --show-toplevel`/sw/chaac_server /var/flaskapp/
sudo rsync --chown=www-data:www-data -a `git rev-parse --show-toplevel`/sw/chaac /var/flaskapp/

cd `git rev-parse --show-toplevel`/sw/chaac_server

echo "Updating nginx config setup"
sudo rm -f /etc/nginx/sites-enabled/default
sudo cp support/nginx_chaac_server /etc/nginx/sites-enabled/chaac_server

sudo rm -f /etc/nginx/nginx.conf
sudo cp support/nginx.conf /etc/nginx/nginx.conf

echo "Updating supervisor config setup"
sudo rm -f /etc/supervisor/conf.d/chaac_server.conf
sudo cp support/chaac_server.conf /etc/supervisor/conf.d/chaac_server.conf

cd -

echo "restart nginx"
sudo systemctl restart nginx

echo "restart supervisor"
sudo systemctl restart supervisor
