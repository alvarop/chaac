#!/bin/bash

echo "Updating files"
sudo rsync --chown=www-data:www-data -a `git rev-parse --show-toplevel`/sw/chaac_frontend /var/flaskapp/
sudo rsync --chown=www-data:www-data -a `git rev-parse --show-toplevel`/sw/chaac /var/

cd `git rev-parse --show-toplevel`/sw/chaac_frontend

sudo pip install -e /var/chaac

echo "Updating nginx config setup"
sudo rm -f /etc/nginx/sites-enabled/default
sudo cp support/nginx_chaac_frontend /etc/nginx/sites-enabled/chaac_frontend

sudo rm -f /etc/nginx/nginx.conf
sudo cp support/nginx.conf /etc/nginx/nginx.conf

echo "Updating supervisor config setup"
sudo rm -f /etc/supervisor/conf.d/chaac_frontend.conf
sudo cp support/chaac_frontend.conf /etc/supervisor/conf.d/chaac_frontend.conf

cd -

echo "restart nginx"
sudo systemctl restart nginx

echo "restart supervisor"
sudo systemctl restart supervisor
