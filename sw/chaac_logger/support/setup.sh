#!/bin/bash

# Make sure we're in the right directory
cd `git rev-parse --show-toplevel`/sw/chaac_logger

echo "Remove old setup"
sudo rm -r /var/chaac

bash ./support/update.sh

cd -
