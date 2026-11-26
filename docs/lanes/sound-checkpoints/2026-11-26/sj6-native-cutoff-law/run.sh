#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd); X="$D/sj6-ota-history/control-admission"; cd "$D"
W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src/dsp; T="$D/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
O=$(mktemp -d "$X/tmp.XXXXXX"); trap 'rm -rf "$O"' EXIT; export TMPDIR="$O"
for n in direct log;do F=(); [[ $n == direct ]] || F=(-DLOG_ROUNDTRIP)
 cc -shared -fPIC -Os -ffp-contract=off -I"$W" "${F[@]}" "$X/candidate.c" -o "$O/$n.so"
 "$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -I"$W" "${F[@]}" -c "$X/candidate.c" -o "$O/$n.o"
 "$T/bin/nm" -S --size-sort "$O/$n.o"
done > "$X/object.txt"
PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench python3 "$X/sweep.py" "$O/direct.so" "$O/log.so" > "$X/sweep.json"
