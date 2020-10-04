#!/bin/bash


echo "Updating files"
cd `git rev-parse --show-toplevel`/sw/lora_mqtt_client

echo "Updating supervisor config setup"
sudo rm -f /etc/supervisor/conf.d/lora_mqtt_client.conf
sudo cp support/lora_mqtt_client.conf /etc/supervisor/conf.d/lora_mqtt_client.conf

cd -

echo "restart supervisor"
sudo systemctl restart supervisor
