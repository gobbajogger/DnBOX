#!/bin/bash
set -euo pipefail
F=$(cd "$(dirname "$0")" && pwd)
D=$(mktemp -d)
trap 'rm -rf "$D"' EXIT
cd "${1:?source root}"
S=firmware/src/dsp
mkdir -p "$D/src/s1c"
cp "$S/fastmath.h" "$D/src/"
cp "$S"/s1c/{s1c.h,s1u.h,s1c_config.h} "$D/src/s1c/"
cp "$S/s1c/s1c.c" "$D/src/s1c/base.c"
patch --batch "$D/src/s1c/base.c" -o "$D/src/s1c/s1c.c" -i "$F/owner48-zero-osc4.patch"
sed -i "/^static __attribute__((noinline)) void osc4_fill/i unsigned lab_osc4_hits;" "$D/src/s1c/s1c.c"
sed -i "s/osc4_fill(s,osc_frac,0,wave_tile);/osc4_fill(s,osc_frac,0,wave_tile);++lab_osc4_hits;/" "$D/src/s1c/s1c.c"
a=(-O2 -std=c11 -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -Ifirmware/src -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c -Ifirmware/src/app)
cc "${a[@]}" -c "$D/src/s1c/base.c" -o "$D/base.o"
cc "${a[@]}" -DDNBOX_S1_OSC4=1 -c "$D/src/s1c/s1c.c" -o "$D/cand.o"
nm -g --defined-only "$D/cand.o" | awk 'NF==3 {print $3,"candidate_"$3}' > "$D/rename.txt"
objcopy --redefine-syms "$D/rename.txt" "$D/cand.o"
cc "${a[@]}" "$F/owner48-zero-check.c" "$D/base.o" "$D/cand.o" "$F/owner48-zero-ima.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -lm -Wl,--gc-sections -o "$D/check"
"$D/check"
