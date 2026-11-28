#!/bin/bash
# usage: check-bank-cache.sh BASELINE_TREE CANDIDATE_TREE
set -eu
A="$1/firmware/src/dsp"; B="$2/firmware/src/dsp"
tmp="$(mktemp -d)"
trap 'rm -rf -- "$tmp"' EXIT
here="$(cd "$(dirname "$0")" && pwd)"
cp "$A/drum808.c" "$tmp/orig.c"
flags=(-std=c99 -ffp-contract=off -O2 "-I$B")
cc "${flags[@]}" -c "$tmp/orig.c" -o "$tmp/orig.o"
mapfile -t names < <(nm -g "$tmp/orig.o" | awk '$2~/^[TDB]$/ {print $3}')
args=(); for i in "${names[@]}"; do args+=(--redefine-sym "$i=orig_$i"); done
objcopy "${args[@]}" "$tmp/orig.o" "$tmp/renamed.o"
cc "${flags[@]}" "$here/owner48-808-check-bank-cache.c" "$tmp/renamed.o" "$B/drum808.c" "$B/x0x_param.c" -lm -o "$tmp/check"
"$tmp/check"
