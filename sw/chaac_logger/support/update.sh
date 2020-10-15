#!/bin/bash


echo "Updating files"

cd `git rev-parse --show-toplevel`/sw/chaac_logger

echo "Updating supervisor config setup"
sudo rm -f /etc/supervisor/conf.d/chaac_logger.conf
sudo cp support/chaac_logger.conf /etc/supervisor/conf.d/chaac_logger.conf

cd -

echo "restart supervisor"
sudo systemctl restart supervisor
