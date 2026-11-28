#!/bin/bash
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
base=$(realpath "${1:?stock source root}")
cand=$(realpath "${2:?patched source root}")
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
cd "$base"
a=(-O2 -std=c11 -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -Ifirmware/src -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c -Ifirmware/src/app)
cu="$cand/firmware/src/dsp/s1c"
cc "${a[@]}" -c "$cu/s1u.c" -o "$T/cu.o"
cc "${a[@]}" -c "$cu/s1c.c" -o "$T/cc.o"
nm -g --defined-only "$T/cu.o" "$T/cc.o" | awk 'NF==3{print $3,"candidate_"$3}' | sort -u >"$T/names"
for o in "$T/cu.o" "$T/cc.o";do objcopy --redefine-syms "$T/names" "$o";done
cc "${a[@]}" "$here/lfo-witness.c" "$T/cu.o" "$T/cc.o" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -lm -Wl,--gc-sections -o "$T/check"
python3 tools/pack_synthone_factory.py "$T/s1.pack"
"$T/check" "$T/s1.pack"
