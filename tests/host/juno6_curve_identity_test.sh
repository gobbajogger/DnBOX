#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${JUNO_CURVE_OUT:-build/host/juno-curve-identity}
BASE=${JUNO_BASE_SOURCE:-}
mkdir -p "$OUT"
# Original authenticated source, not a handwritten reimplementation.
if [ -z "$BASE" ]; then
    git show 94197fc839b00aff47b98ca02f3c6396e4a6eabf:firmware/src/dsp/juno6.c > "$OUT/base.c"
    BASE="$OUT/base.c"
fi
DEFS=
for f in init note_on note_off all_off render chorus_init chorus_set chorus_process active set; do DEFS="$DEFS -Djuno6_$f=ref_juno6_$f"; done
for opt in O2 Os; do
    ${CC:-cc} -std=c99 -"$opt" -ffp-contract=off -fno-fast-math -DX0X_HOST -Ifirmware/src/dsp $DEFS -c "$BASE" -o "$OUT/base-$opt.o"
    ${CC:-cc} -std=c99 -"$opt" -ffp-contract=off -fno-fast-math -DX0X_HOST -Wall -Wextra -Werror -Ifirmware/src/dsp ${SAN_FLAGS:-} \
        tests/host/juno6_curve_identity_test.c firmware/src/dsp/juno6.c "$OUT/base-$opt.o" -lm -o "$OUT/test-$opt"
    "$OUT/test-$opt"
done
