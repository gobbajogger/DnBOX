#!/usr/bin/env bash
set -eu
source_tree=$(realpath "$1")
out=$(realpath -m "$2")
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$out"
cd "$source_tree"
flags=(-O2 -std=c11 -DX0X_HOST -DDNBX_DIAGNOSTIC_COUNTERS=0 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -Ifirmware/src -Ifirmware/src/app -Iexperiments/original_synth)
units=("$here/witness.c" firmware/src/app/{original_runtime,original_sound,s1_dry_part}.c firmware/src/dsp/{juno6,s1_dry_voice,dnbx_s1_envelope}.c)
if test -f firmware/src/dsp/s1_band_morph.c;then
    units+=(firmware/src/dsp/s1_band_morph.c)
    flags+=(-DTEST_MORPH)
fi
objs=()
for u in "${units[@]}";do
    o="$out/$(basename "$u" .c).o";cc "${flags[@]}" -c "$u" -o "$o";objs+=("$o")
done
ld -r "${objs[@]}" -o "$out/combined.o"
{ echo '#include <stdlib.h>';nm -u "$out/combined.o" | awk '$2 ~ /^(original_(chicago|miami)_)/ {print "void "$2"(void){abort();}"}'; } > "$out/traps.c"
cc "${flags[@]}" -c "$out/traps.c" -o "$out/traps.o"
cc -Wl,--gc-sections "$out/combined.o" "$out/traps.o" -lm -o "$out/witness"
