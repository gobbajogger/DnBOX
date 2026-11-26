#!/usr/bin/env bash
set -euo pipefail
W=$(realpath "$(dirname "$0")")
P=$W/..
D=$(realpath "$1")
C=$P/sj6-crusher-converter
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
python3 "$W/draft.py" "$D/firmware/src/app" "$T" | diff - "$W/pins.txt"
for arm in old new; do
 f=()
 [ "$arm" = old ] || f=(-DSJ6_RATE_CARRIER_WITNESS -I"$T")
 cc -O2 -ffp-contract=off -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ORIGINAL_SYNTH=1 $(cat "$P/all-controls-remote-join/compile-flags.txt") "${f[@]}" -I"$C" -I"$D/firmware/src" -I"$D/firmware/src/app" -I"$D/firmware/src/dsp" -I"$D/build/smk37-elite/gen" -I"$D/experiments/original_synth" -I"$D/experiments/wobble" "$W/validate.c" -o "$T/validate"
 "$T/validate"
done
python3 "$W/protocol.py" "$D/firmware/src/app" "$T"
