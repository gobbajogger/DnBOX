#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Takes the existing exact build's generated inputs and writes only to OUT.
set -eu
cd "$(dirname "$0")/../.."
OUT=${1:-$(mktemp -d "${TMPDIR:-/tmp}/dnbox-sj6-wide.XXXXXX")}
mkdir -p "$OUT"
GEN=${X0X_HOST_GENERATED_DIR:-$PWD/build/gen}
for name in felucca_font.h x0x_drum_samples.h x0x_drum_tables.h x0x_break_bank.h; do
    test -f "$GEN/$name" || { echo "Missing generated host input: $GEN/$name" >&2; exit 2; }
done
export X0X_SUPERJ6_FACTORY=1
export PYTHONDONTWRITEBYTECODE=1
. tests/host/original_build.sh
DARWIN=
[ "$(uname -s)" != Darwin ] || DARWIN=-D_DARWIN_C_SOURCE
# The linker wraps capture addresses and forward calls to the actual DSP.
# shellcheck disable=SC2086
${CC:-cc} -std=c99 -O1 -D_POSIX_C_SOURCE=200809L $DARWIN -DX0X_HOST \
    -DX0X_SYNTHONE_PATCH=1 -DX0X_SUPERJ6_STOCK_PRESET=1 \
    -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -I"$GEN" \
    $ORIGINAL_FLAGS ${SAN_FLAGS:-} tests/host/superj6_stock_wide_bridge_test.c \
    $ORIGINAL_SOURCES -Wl,--wrap=juno6_init -Wl,--wrap=juno6_chorus_init -lm \
    -o "$OUT/test" > "$OUT/compile.log" 2>&1 || { cat "$OUT/compile.log" >&2; exit 1; }
"$OUT/test"
