#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D")
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
cp -a "$R/ui-canvas-first/donor-tree/tests" "$D/tree/tests"
cd "$D/tree"
export OUT="$T"
sh tests/host/smk_clip_knob_test.sh --extract-only
python3 tools/gen_font.py "$T/felucca_font.h"
python3 tests/host/prepare_clip_keys_render.py "$T"
python3 "$D/verify-prepare.py" "$T"
cc '-DSMK_MODEL="SMK-37 Pro"' -std=gnu99 -O1 -DX0X_UI_HOME_STRIP_SPIKE=1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/hal -I"$T" "$D/verify-render.c" firmware/src/app/smk_clip_arp_share.c -lm -o "$T/test"
"$T/test"
