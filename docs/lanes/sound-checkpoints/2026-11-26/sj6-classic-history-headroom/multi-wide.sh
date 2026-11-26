#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); R=$(dirname "$D")
BASE=${1:-$R/sj6-hp-connected-zero/host-donor}
T=$(mktemp -d); trap 'rm -r "$T"' EXIT
mkdir -p "$T/firmware/src/dsp"
cp "$BASE/firmware/src/dsp/"dnbx_core.{c,h} "$T/firmware/src/dsp/"
(cd "$T";sha256sum -c "$D/preimages.txt";patch -s --fuzz=0 -p1 < "$D/candidate.patch")
cp -as "$BASE/firmware/src/dsp/." "$T/firmware/src/dsp" 2>/dev/null || true
mkdir "$T/firmware/src/app"; cp -as "$BASE/firmware/src/app/." "$T/firmware/src/app/"
python3 "$D/chorus-q15-wider.py" "$T/firmware/src/dsp/juno6.c"
F=(-std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1)
S="$BASE/firmware/src"; X="$BASE/experiments/original_synth"
for mode in 0 1; do
cc "${F[@]}" -DRING_ENC=4096.f -DDNBX_HOST_SJ6_FIXED_SOURCE_ROUTE=$mode -I"$T/firmware/src/dsp" -I"$T/firmware/src/app" -I"$S/dsp" -I"$S" -I"$X" "$D/connected-multi.c" "$R/sj6-cutoff-firmware/unused-links.c" "$T/firmware/src/dsp/juno6.c" "$S/dsp/master.c" "$T/firmware/src/dsp/dnbx_core.c" -lm -Wl,--gc-sections -o "$T/probe"
"$T/probe"
done
