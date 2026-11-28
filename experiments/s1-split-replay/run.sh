#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail
cd "$(dirname "$0")/../.."
d=$(mktemp -d)
trap 'rm -rf -- "$d"' EXIT
p=experiments/s1-split-replay
cp firmware/src/dsp/s1c/s1c.c "$d/base.c"
cp "$d/base.c" "$d/s1c.c"
patch --silent -d "$d" -p1 < "$p/serial-tape.patch"
flags=(-O2 -std=c11 -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -Ifirmware/src -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c -Ifirmware/src/app)
cc "${flags[@]}" -c "$d/base.c" -o "$d/base.o"
cc "${flags[@]}" -c "$d/s1c.c" -o "$d/split.o"
nm -g --defined-only "$d/split.o" | awk 'NF==3 {print $3,"split_"$3}' > "$d/rename.txt"
objcopy --redefine-syms "$d/rename.txt" "$d/split.o"
cc "${flags[@]}" "$p/check.c" "$d/base.o" "$d/split.o" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -lm -Wl,--gc-sections -o "$d/check"
"$d/check"
