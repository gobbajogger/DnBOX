#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# Isolated current-layout receiver; does not write project state or enable firmware features.
set -euo pipefail
cd "$(dirname "$0")"
here=$PWD
root=$(cd ../.. && pwd)
tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT
mkdir -p "$tmp/firmware/src/dsp"
cp "$root/firmware/src/dsp/juno6.h" "$tmp/firmware/src/dsp/"
patch -s -d "$tmp" -p1 < juno6.patch
"${CC:-cc}" -std=c11 -O2 -ffp-contract=off -fsanitize=undefined -fno-sanitize-recover=all \
    -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SJ6_NATIVE_DELAY=1 \
    -I"$tmp/firmware/src/dsp" -I"$root/firmware/src/dsp" "$here/test.c" -o "$tmp/check"
"$tmp/check"
