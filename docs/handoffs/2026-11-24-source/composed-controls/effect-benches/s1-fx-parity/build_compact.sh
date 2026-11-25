#!/usr/bin/env bash
set -euo pipefail
W=$(realpath "$(dirname "$0")")
SRC=$(realpath "${1:-$W/../fw}")
O=$(realpath "$2")
cd "$SRC"
# Extra compile flags are positional and optional; the checked-in stock
# fixture explicitly uses no recovered/full-candidate flags.
shift 2
cc -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 \
  -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen "$@" \
  "$W/compact.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c \
  -Wl,--gc-sections -lm -o "$O"
