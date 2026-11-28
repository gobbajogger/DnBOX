#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=$(mktemp -d "${TMPDIR:-/tmp}/s1c-delay-wrap.XXXXXX")
trap 'rm -rf "$out"' EXIT HUP INT TERM
${CC:-cc} -std=c11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections \
    -Ifirmware/src -include firmware/src/dsp/s1c/s1c_config.h \
    tests/host/s1c_delay_wrap_test.c firmware/src/dsp/s1c/s1u.c \
    -Wl,--gc-sections -lm -o "$out/test"
"$out/test"
