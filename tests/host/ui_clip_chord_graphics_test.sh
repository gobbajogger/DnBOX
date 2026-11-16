#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${OUT:-build/host/clip-chord-graphics}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
export OUT
sh tests/host/ui_clip_keys_render_test.sh > "$OUT/setup.log" 2>&1
mkdir -p "$OUT/unobserved"
cp "$OUT"/*.rgb565be "$OUT/unobserved/"
python3 tests/host/prepare_clip_chord_graphics.py "$OUT"
${CC:-cc} '-DSMK_MODEL="SMK-37 Pro"' -std=gnu99 -O1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/hal -I"$OUT" tests/host/ui_clip_keys_render_test.c firmware/src/app/smk_clip_arp_share.c -lm -o "$OUT/observed-render"
(cd "$OUT" && ./observed-render > observer-render.log)
for raster in "$OUT/unobserved/"*.rgb565be; do cmp "$raster" "$OUT/$(basename "$raster")"; done
${CC:-cc} '-DSMK_MODEL="SMK-37 Pro"' -std=gnu99 -O1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/hal -I"$OUT" "$OUT/chord-real.c" firmware/src/app/smk_clip_arp_share.c -lm -o "$OUT/chord-real"
(cd "$OUT" && ./chord-real)
${CC:-cc} '-DSMK_MODEL="SMK-37 Pro"' -std=gnu99 -O1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/hal -I"$OUT" tests/host/ui_clip_strip_domains_test.c firmware/src/app/smk_clip_arp_share.c -lm -o "$OUT/strip-domains"
(cd "$OUT" && ./strip-domains)
