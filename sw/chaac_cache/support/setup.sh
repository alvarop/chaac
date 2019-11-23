#!/bin/bash

# Make sure we're in the right directory
cd `git rev-parse --show-toplevel`/sw/chaac_cache

bash ./support/update.sh

cd -
