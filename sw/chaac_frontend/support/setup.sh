#!/bin/bash

# Make sure we're in the right directory
cd `git rev-parse --show-toplevel`/sw/chaac_frontend

echo "Remove old setup"
sudo rm -r /var/flaskapp/chaac_frontend

bash ./support/update.sh

cd -

