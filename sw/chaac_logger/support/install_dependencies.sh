#!/bin/bash

sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install python3-pip supervisor git python3-numpy -y
sudo pip3 install pipenv