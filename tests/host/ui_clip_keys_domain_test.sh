#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${OUT:-build/host/clip-key-domains}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
export OUT
sh tests/host/ui_clip_keys_render_test.sh > "$OUT/setup.log" 2>&1
mkdir -p "$OUT/unobserved"
cp "$OUT"/*.rgb565be "$OUT/unobserved/"
cp tests/host/ui_clip_keys_text_observer.inc "$OUT/"
python3 - "$OUT" <<'PY'
import sys
from pathlib import Path
p = Path(sys.argv[1]) / 'actual-clip-key-host.inc'
s = p.read_text()
anchor = '#include "gfx.c"'
assert s.count(anchor) == 1
p.write_text(s.replace(anchor, anchor + '\n#include "ui_clip_keys_text_observer.inc"'))
PY
${CC:-cc} '-DSMK_MODEL="SMK-37 Pro"' -std=gnu99 -O1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/hal -I"$OUT" tests/host/ui_clip_keys_render_test.c firmware/src/app/smk_clip_arp_share.c -lm -o "$OUT/observed-render"
(cd "$OUT" && ./observed-render > observer-render.log)
for raster in "$OUT/unobserved/"*.rgb565be; do cmp "$raster" "$OUT/$(basename "$raster")"; done
${CC:-cc} '-DSMK_MODEL="SMK-37 Pro"' -std=gnu99 -O1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/hal -I"$OUT" tests/host/ui_clip_keys_domain_test.c firmware/src/app/smk_clip_arp_share.c -lm -o "$OUT/domains"
(cd "$OUT" && ./domains > domains.log)
python3 - "$OUT" <<'PY_RECEIPT'
import hashlib, json, os, sys
from pathlib import Path
out = Path(sys.argv[1])
paths = ['tests/host/ui_clip_keys_domain_test.c', 'tests/host/ui_clip_keys_domain_test.sh', 'tests/host/ui_clip_keys_text_observer.inc']
receipt = {'schema': 'dnbox-clip-domain-inputs/1', 'inputs': {}, 'generated': {}, 'setupProvenanceSha256': hashlib.sha256((out / 'provenance.json').read_bytes()).hexdigest()}
for name in paths:
    receipt['inputs'][name] = hashlib.sha256(Path(name).read_bytes()).hexdigest()
for p in sorted(out.glob('*.inc')) + [out / 'felucca_font.h']:
    receipt['generated'][p.name] = hashlib.sha256(p.read_bytes()).hexdigest()
receipt['compileCommand'] = [os.environ.get('CC', 'cc'), '-DSMK_MODEL="SMK-37 Pro"', '-std=gnu99', '-O1', '-Ifirmware/src', '-Ifirmware/src/app', '-Ifirmware/hal', '-I$OUT', 'tests/host/ui_clip_keys_domain_test.c', 'firmware/src/app/smk_clip_arp_share.c', '-lm', '-o', '$OUT/domains']
receipt['scope'] = 'Final instrumented include, not original setup include. Original provenance.json is retained separately.'
(out / 'domain-provenance.json').write_text(json.dumps(receipt, indent=2) + '\n')
PY_RECEIPT
tail -n 1 "$OUT/domains.log"
echo 'PASS text observer leaves all 18 baseline rasters byte-identical; fixed context, not all combinations or visual acceptance'
