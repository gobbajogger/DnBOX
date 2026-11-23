#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# REVERB bank DSP mapping: Low Cut on the wet input, Return on the wet output only.
# Then two source mutants (low cut skipped; return scaling dry) must each fail.
set -eu
cd "$(dirname "$0")/../.."
OUT=${OUT:-build/host}/reverb_bank_mapping; mkdir -p "$OUT"
build() { ${CC:-cc} -std=c99 -O2 -w -ffp-contract=off -Ifirmware/src/dsp -Ifirmware/src -I"$OUT" \
    tests/host/reverb_bank_mapping_test.c "$1" firmware/src/dsp/drum909.c firmware/src/dsp/x0x_param.c -lm -o "$2"; }
python3 tools/gen_drum_samples.py "$OUT/x0x_drum_samples.h" > "$OUT/generate.log" 2>&1
build firmware/src/dsp/fxbus.c "$OUT/test"
"$OUT/test"
mut() {
    python3 - "$OUT/m.c" "$1" "$2" <<'PY'
import sys
s=open('firmware/src/dsp/fxbus.c').read(); a,b=sys.argv[2],sys.argv[3]
assert a in s, a
open(sys.argv[1],'w').write(s.replace(a,b,1))
PY
    sed -i 's|#include "|#include "'"$PWD"'/firmware/src/dsp/|' "$OUT/m.c"
    build "$OUT/m.c" "$OUT/mut"
    if "$OUT/mut" > "$OUT/mut.log"; then echo "MUTANT SURVIVED: $3"; exit 1; fi
    echo "mutant killed: $3"
}
mut 'const float x = d9_biquad_tick(&f->rv_hp, in[i]);' 'const float x = in[i];' "low cut skipped"
mut '        out[i] += y * lvl;' '        out[i] = (out[i] + y) * lvl;' "return scales dry"
echo "reverb-bank-mapping: mutants killed"
