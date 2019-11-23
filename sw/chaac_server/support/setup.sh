#!/bin/bash

# Make sure we're in the right directory
cd `git rev-parse --show-toplevel`/sw/chaac_server

echo "Copy config file to /etc/chaac (renamed current config to *.old)"
sudo mkdir -p /etc/chaac
sudo mv /etc/chaac/chaac_cfg.yml /etc/chaac/chaac_cfg.yml.old
sudo cp support/chaac_cfg.yml /etc/chaac/chaac_cfg.yml

echo "Remove old setup"
sudo rm -r /var/flaskapp/chaac_server
sudo rm -r /var/chaac

echo "Updating nginx config setup"
echo "NOTE: This will get rid of all certbot fun..."
sudo rm -f /etc/nginx/sites-enabled/default
sudo mv /etc/nginx/sites-available/chaac_server /etc/nginx/sites-available/chaac_server.old
sudo cp support/nginx_chaac_server /etc/nginx/sites-available/chaac_server
sudo ln -s /etc/nginx/sites-available/chaac_server /etc/nginx/sites-enabled/

sudo rm -f /etc/nginx/nginx.conf
sudo cp support/nginx.conf /etc/nginx/nginx.conf

bash ./support/update.sh

cd -

