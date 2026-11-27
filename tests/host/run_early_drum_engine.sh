#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=${EARLY_D9_OUT:-build/host/early-d9}
mkdir -p "$out"
sh host/build_host.sh --generate-only >"$out/generate.log" 2>&1
. tests/host/original_build.sh
${CC:-cc} -O1 -g -std=c11 -ffp-contract=off -Wall -Wextra -Wno-unused-function -Wno-unused-parameter \
    -DX0X_HOST -DX0X_D9_EARLY_HIT=1 -D_POSIX_C_SOURCE=200809L \
    -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/early_drum_engine_test.c $ORIGINAL_FLAGS $ORIGINAL_SOURCES -lm -o "$out/test" >"$out/compile.log" 2>&1
"$out/test"
