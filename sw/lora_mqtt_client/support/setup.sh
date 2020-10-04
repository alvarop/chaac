#!/bin/bash

# Make sure we're in the right directory
cd `git rev-parse --show-toplevel`/sw/lora_mqtt_client

echo "Remove old setup"

bash ./support/update.sh

cd -
