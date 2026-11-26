#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd);cd "$D"
B=${1:-../sj6-hp-connected-zero/host-donor/firmware/src/dsp}
B=$(cd "$B"&&pwd)
T=$(mktemp -d);trap 'rm -r "$T"' EXIT
pin=$(cut -d' ' -f1 donor.pin);echo "$pin  $B/dnbx_core.c"|sha256sum -c
cp "$B/dnbx_core.c" "$T/candidate.c"
python3 patch.py "$T/candidate.c"
sed -i '/if (osc_gain\[l\]!=1.f) x\*=osc_gain\[l\];/a\                extern float carrier_tap[2][128];\n                carrier_tap[l][k]+=velocity*amp*x;' "$T/candidate.c"
for variant in '32,7,8,1' '64,12,8,1' '64,16,16,1';do
 IFS=, read -r N H Q M <<< "$variant"
 O="$D/test-sparse-$N-$H-$Q-$M";mkdir -p "$O"
 python3 generate-sparse.py "$N" "$H" "$Q" > "$T/family-data.h"
 cc -O2 -std=c99 -ffp-contract=off -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DDNBX_WAVE_FAMILY="$M" -DFAMILY_WORD="$Q" -I"$B" -I"$D" -I"$T" "$T/candidate.c" probe.c -lm -o "$T/run"
 (cd "$O"; "$T/run" > log.txt)
 cp "$T/family-data.h" "$O/"
done
