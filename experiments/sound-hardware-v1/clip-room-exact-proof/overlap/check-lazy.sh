#!/bin/bash
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
base=$(realpath "${1:?stock source root}")
cand=$(realpath "${2:?patched source root}")
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
cd "$base"
a=(-DDNBOX_S1_SKIP_FIXED=1 -DDNBOX_S1_LAZY_PEER=1 -O2 -std=c11 -DX0X_HOST -DDNBOX_S1_WORK_SPLIT=1 "-DDNBOX_S1_SPLIT_START=${WORK_SPLIT_START:-2}" "-DS1C_SPLIT_SPAN=${WORK_TILE:-128}u" -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -Ifirmware/src -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c -Ifirmware/src/app)
cu="$cand/firmware/src/dsp/s1c"
cc "${a[@]}" -c "$cu/s1u.c" -o "$T/cu.o"
cc "${a[@]}" -c "$cu/s1c.c" -o "$T/cc.o"
nm -g --defined-only "$T/cu.o" "$T/cc.o" | awk 'NF==3{print $3,"candidate_"$3}' | sort -u >"$T/names"
for o in "$T/cu.o" "$T/cc.o";do objcopy --redefine-syms "$T/names" "$o";done
cc "${a[@]}" "$here/check-lazy.c" "$T/cu.o" "$T/cc.o" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -lm -Wl,--gc-sections -o "$T/check"
python3 tools/pack_synthone_factory.py "$T/s1.pack"
"$T/check" "$T/s1.pack"
