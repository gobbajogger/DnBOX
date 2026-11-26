#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); cd "$D"
B=${1:-../sj6-hp-connected-zero/host-donor/firmware/src/dsp}
B=$(cd "$B" && pwd)
pin=$(cut -d' ' -f1 donor.pin); echo "$pin  $B/dnbx_core.c"|sha256sum -c
T=$(mktemp -d); trap 'rm -r "$T"' EXIT
cp "$B/dnbx_core.c" "$T/candidate.c"
sed -i '/if (osc_gain\[l\]!=1.f) x\*=osc_gain\[l\];/a\                extern float diagnostic_source[2][128];\n                diagnostic_source[l][k]+=velocity*amp*x;' "$T/candidate.c"
python3 source-ablation.py "$T/candidate.c"
for v in 0 1 2 3; do
 cc -O2 -std=c99 -ffp-contract=off -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DRAW_SAW_ABLATION=$v -I"$B" -I"$D" "$T/candidate.c" run.c -lm -o "$T/run"
 "$T/run" > "ablation-$v.txt"
done
