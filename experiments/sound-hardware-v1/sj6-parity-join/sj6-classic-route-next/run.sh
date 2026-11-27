#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
base="$PWD/sj6-normal-shared-state/tree/firmware/src"
cc -std=gnu11 -O2 -Wall -Wextra -Werror -ffp-contract=off -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NATIVE_CLASSIC=1 -I"$base" -I"$base/dsp" -shared -fPIC sj6-classic-route-next/bridge.c -lm -o sj6-classic-route-next/candidate.so
PYTHONPATH=/tmp/sj6-room-next-py:$PWD/ab-effects-readonly/sj6-fullbench/bench python sj6-classic-route-next/check.py "$@"
