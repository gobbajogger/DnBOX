#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd)
T=/tmp/phaser-warm-own.Y0XNbQ/tree
s="$T/firmware/src"
src="$s/dsp/drum909.c"
printf '%s  %s\n' 69e6f18b56ef6c710ed018dd352d5c79eac5faabc1fe653b40606287c8a6c5d7 "$src" | sha256sum -c -
w=$(mktemp -d /tmp/merge-leaf.XXXX);trap 'rm -r "$w"' EXIT
cc -Os -ffp-contract=off -ffunction-sections -fdata-sections -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1 -DX0X_TANH_SPLIT_LAB=1 -I"$s/dsp" -I"$s" -I"$T/build/smk37-elite/gen" "$D/probe.c" "$src" "$s/dsp/x0x_param.c" -Wl,--gc-sections -lm -o "$w/probe"
printf 'voice,first_differing_frame,different_bus_words,different_prefix_words,first_reference_bits,first_merged_bits,exact_whole_state\n'
"$w/probe"
