#!/bin/bash
set -euo pipefail
D=$PWD/sj6-raw-main-ota
W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src
O=$(mktemp -d "$D/leaf.XXXXXX"); trap 'rm -rf "$O"' EXIT
python3 "$D/make.py" "$O/f.c"
cc -O2 -fPIC -shared -ffp-contract=off -DX0X_SJ6_OTA_NATIVE=1 -I"$W/dsp" -I"$W" "$O/f.c" -lm -o "$O/f.so"
cc -O2 -fPIC -shared -ffp-contract=off "$D/physical.c" -lm -o "$O/p.so"
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
python3 "$D/physical_score.py" "$O/f.so" "$O/p.so" "$@"
