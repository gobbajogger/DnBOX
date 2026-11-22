#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
exec python3 tests/host/smc_caption_geometry_test.py
