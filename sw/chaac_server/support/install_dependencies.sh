#!/bin/bash

# sudo apt-get update
# sudo apt-get upgrade -y
sudo apt-get install python3-pip nginx supervisor git python3-numpy -y
sudo pip3 install pipenv flask gunicorn requests pyyaml

echo "Please reboot or log out/in for changes to take effect"
echo "Don't forget to run raspi-config to change timezone to correct one"
