#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
T=$(cd "${1:-$D/../guarded-fit-near-bound/tree}" && pwd)
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
for f in 0 1; do
  "${CC:-cc}" -std=c99 -O2 -ffp-contract=off -DX0X_909_HUFF_DIAG=$f -DX0X_COLD_COMPANION_LAB=$f \
    "-I$T/firmware/src" "-I$T/firmware/src/dsp" "-I$T/build/smk37/gen" "$D/909-stress.c" \
    "$T/firmware/src/dsp/x0x_param.c" -lm -o "$W/$f"
  "$W/$f" > "$W/$f.f32"
done
cmp "$W"/{0,1}.f32
sha256sum "$W/0.f32"
