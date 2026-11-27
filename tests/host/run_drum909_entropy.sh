#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# Compare the real drum renderer's four buses and the entire IMA predictor/read position trace.
set -euo pipefail
cd "$(dirname "$0")/../.."
out=$(mktemp -d)
trap 'rm -rf -- "$out"' EXIT
cc=${CC:-cc}
python3 tools/gen_drum_samples.py "$out/x0x_drum_samples.h"
python3 tools/gen_drum_entropy.py "$out/x0x_drum_samples.h" "$out/x0x_drum_entropy.h"
for mode in 0 1; do
    "$cc" -O2 -std=c99 -DX0X_HOST -DDNBOX_LAB_D9_SUFFIX=1 "-DX0X_D9_ENTROPY=$mode" \
        -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror -Wno-unused-function \
        -Ifirmware/src -Ifirmware/src/dsp -I"$out" tests/host/drum909_entropy_witness.c \
        firmware/src/dsp/drum909.c firmware/src/dsp/x0x_param.c -lm -o "$out/arm$mode"
    "$out/arm$mode" | sha256sum > "$out/arm$mode.sha"
done
cmp "$out/arm0.sha" "$out/arm1.sha"
printf 'raw and entropy traces match: '
cat "$out/arm1.sha"
