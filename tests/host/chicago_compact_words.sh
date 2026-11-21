#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/chicago-compact-words}
mkdir -p "$out"
original=experiments/original_synth
python3 tools/gen_chicago_compact.py "$original/chicago_packed_private.h" "$out/chicago_compact_private.h"
${CC:-cc} -std=c99 -O2 -DX0X_CHICAGO_COMPACT_HEADERS -I"$out" -I"$original" \
    tests/host/chicago_compact_words.c "$original/chicago_packed.c" "$original/lossless_table.c" -o "$out/test"
"$out/test"
