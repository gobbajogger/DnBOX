#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail
cd "$(dirname "$0")/../.."
tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT
[ -f build/gen/x0x_drum_tables.h ] || sh host/build_host.sh --generate-only >/dev/null
common=(-O2 -std=c11 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections \
        -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -I. -Ifirmware/src -Iexperiments/original_synth -Iexperiments/wobble -Ibuild/gen)
"${CC:-cc}" "${common[@]}" -c firmware/src/app/original_runtime.c -o "$tmp/off.o"
"${CC:-cc}" "${common[@]}" -DX0X_SJ6_NATIVE_DELAY=1 -Wl,--gc-sections \
    experiments/sj6-delay-receiver/boundary.c firmware/src/app/original_runtime.c -lm -o "$tmp/witness"
"$tmp/witness"
"${CC:-cc}" "${common[@]}" -DX0X_SJ6_NATIVE_DELAY=1 -Wl,--gc-sections \
    experiments/sj6-delay-receiver/owned.c firmware/src/app/original_runtime.c -lm -o "$tmp/owned"
"$tmp/owned"
