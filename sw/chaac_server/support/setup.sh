#!/bin/bash

# Make sure we're in the right directory
cd `git rev-parse --show-toplevel`/sw/chaac_server

echo "pipenv setup"

# Temporary workaround. Getting hash errors when trying to install without --sequential
# also getting modules not found error when trying to install from pipfile after
# it's moved to /var :(

rm ./Pipfile
export PIPENV_VENV_IN_PROJECT=true; pipenv install --three
export PIPENV_VENV_IN_PROJECT=true; pipenv install flask gunicorn requests

echo "Remove old setup"
sudo rm -r /var/flaskapp/chaac_server
sudo rm -r /var/flaskapp/chaac

echo "Updating nginx config setup"
echo "NOTE: This will get rid of all certbot fun..."
sudo rm -f /etc/nginx/sites-enabled/default
sudo cp support/nginx_chaac_server /etc/nginx/sites-available/chaac_server
sudo ln -s /etc/nginx/sites-available/chaac_server /etc/nginx/sites-enabled/

sudo rm -f /etc/nginx/nginx.conf
sudo cp support/nginx.conf /etc/nginx/nginx.conf


bash ./support/update.sh

cd -

