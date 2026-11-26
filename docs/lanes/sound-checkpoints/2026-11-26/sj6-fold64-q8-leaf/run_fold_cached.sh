#!/usr/bin/env bash
set -euo pipefail
D=$(realpath "$(dirname "$0")");cd "$D/.."
B=$(realpath sj6-hp-connected-zero/host-donor/firmware/src/dsp)
echo "d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625  $B/dnbx_core.c"|sha256sum -c
T=$(mktemp -d); echo "$T"
PYTHONPATH=sj6-spectrum-anchor python "$D/fold.py" sj6-spectrum-anchor/a4.private.json "${1:-64}" "${2:-8}" >"$T/fold-data.h"
cp "$B/dnbx_core.c" "$T/typed.c"
python "$D/wire-fold-cached.py" "$T/typed.c"
for v in 0 1;do
 cc -O2 -std=c99 -ffp-contract=off -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DSJ6_WAVE_PROBE=$v -DPACK=1 -I"$T" -I"$B" -I"$D" -Isj6-wavetable-scope -Isj6-spectrum-anchor "$T/typed.c" "$D/donor_probe.c" -lm -o "$T/$v"
 mkdir "$T/arm-$v";(cd "$T/arm-$v";"$T/$v" >result.txt)
done
sha256sum "$T"/arm-{0,1}/test-*-{2,3}-0.pcm|sort -k2
