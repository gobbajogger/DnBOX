#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${OUT:-build/host/actual-clip-keys}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
export OUT
SRC=firmware/src sh tests/host/smk_clip_knob_test.sh --extract-only
python3 tools/gen_font.py "$OUT/felucca_font.h"
python3 tests/host/prepare_clip_keys_render.py "$OUT"
${CC:-cc} '-DSMK_MODEL="SMK-37 Pro"' -std=gnu99 -O1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/hal -I"$OUT" tests/host/ui_clip_keys_render_test.c firmware/src/app/smk_clip_arp_share.c -lm -o "$OUT/render"
(cd "$OUT" && ./render)
