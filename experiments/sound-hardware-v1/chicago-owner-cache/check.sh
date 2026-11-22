#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Run after a normal build has generated build/gen/chicago_compact_private.h.
set -eu
root=$(git rev-parse --show-toplevel)
src="$root/experiments/original_synth"
here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
base=0ab227e2f727a9240e7df0ef972c062b5512357e
git -C "$root" show "$base:experiments/original_synth/chicago_packed.c" > "$work/original.c"
cc -O2 -std=c99 -I"$src" -Doriginal_chicago_packed_lookup=baseline_lookup \
    -c "$work/original.c" -o "$work/original.o"
for mode in full compact; do
    case "$mode" in compact) set -- -DX0X_CHICAGO_COMPACT_HEADERS -I"$root/build/gen";; full) set --;; esac
    cc -O2 -std=c99 -pthread -I"$src" "$@" "$src/chicago_packed.c" \
        "$src/lossless_table.c" "$here/check.c" "$work/original.o" -o "$work/check"
    "$work/check"
done
