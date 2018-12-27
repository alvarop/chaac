#!/bin/bash

# RUN FROM chaac_frontend/ dir

echo "nginx config setup"
sudo rm /etc/nginx/sites-enabled/default
sudo cp support/nginx_chaac_frontend /etc/nginx/sites-enabled/chaac_frontend

echo "supervisor config setup"
sudo rm /etc/supervisor/conf.d/chaac_frontend.conf
sudo cp support/chaac_frontend.conf /etc/supervisor/conf.d/chaac_frontend.conf

echo "pipenv setup"

# Temporary workaround. getting hash errors when trying to install from pipfile :()
rm Pipfile
rm Pipfile.lock

export PIPENV_VENV_IN_PROJECT=true; pipenv install --three
export PIPENV_VENV_IN_PROJECT=true; pipenv install flask gunicorn

echo "copying to /var/"
sudo rm -r /var/flaskapp/chaac_frontend
sudo mkdir -p /var/flaskapp/chaac_frontend/
sudo cp -r . /var/flaskapp/chaac_frontend/

cd /var/flaskapp/chaac_frontend

echo "changing ownership"
sudo chown -R www-data:www-data /var/flaskapp/chaac_frontend

cd -

echo "restart nginx"
sudo systemctl restart nginx

echo "restart supervisor"
sudo systemctl restart supervisor
