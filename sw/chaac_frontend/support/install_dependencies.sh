#!/bin/bash

sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install python3-pip nginx supervisor git -y
sudo pip3 install pipenv

# Make sure www-data can read db file created by pi user
sudo usermod -a -G www-data pi

echo "Please reboot or log out/in for changes to take effect"
echo "Don't forget to run raspi-config to change timezone to correct one"
