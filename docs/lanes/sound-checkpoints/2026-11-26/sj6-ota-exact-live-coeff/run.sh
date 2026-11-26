#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."; D=$PWD; X="$D/sj6-ota-history/setup-state"
T="$D/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
O=$(mktemp -d "$X/tmp.XXXXXX"); trap 'rm -rf "$O"' EXIT; export TMPDIR="$O"
cc -Os -fPIC -shared -ffp-contract=off "$X/setup.c" -lm -o "$O/host.so"
PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench python3 "$X/verify.py" "$O/host.so" | tee "$X/host.json"
"$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -c "$X/setup.c" -o "$O/native.o"
{ "$T/bin/nm" -S "$O/native.o"; "$T/bin/nm" -u "$O/native.o"; } | tee "$X/native.txt"
