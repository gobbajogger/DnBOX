#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""G-FINAL pixel parity: firmware ui_gfinal.h against the reference renderer.

1. Re-render every committed case with tools/gfinal/gfinal.py and require each
   1x compact PNG to hash to tools/gfinal/reference-hashes.json (reference frozen).
2. Render the same cases with firmware/src/app/ui_gfinal.h (binary32, as on the
   device) and require identical 4x4 coverage per role. The reference sees live
   inputs quantised to binary32, exactly what the firmware receives.
3. Static check: every glyph must change over its value/state sweep, and every
   LIVE glyph must change over its live frames.
"""
import hashlib, io, json, os, subprocess, sys, tempfile
from pathlib import Path
import numpy as np
ROOT = Path(__file__).resolve().parents[2]
REF = ROOT / 'tools' / 'gfinal'
sys.path.insert(0, str(REF))
import gfinal  # noqa: E402

def f32(v): return float(np.float32(v)) if isinstance(v, float) else v

def main():
    cases = json.load(open(REF / 'cases.json'))
    hashes = json.load(open(REF / 'reference-hashes.json'))
    ids = list(gfinal.GLYPHS)
    bad_ref = 0
    with tempfile.TemporaryDirectory() as td:
        td = Path(td); exp = bytearray(); lines = []
        for c in cases:
            im = gfinal.render(c['glyph'], c['value'], c['live'] or None, state=c['state'])
            b = io.BytesIO(); im.save(b, format='PNG')
            key = f"frames/1x/{c['glyph']}-compact-{c['case']}.png"
            if hashlib.sha256(b.getvalue()).hexdigest() != hashes.get(key):
                bad_ref += 1; print('REFERENCE DRIFT', key)
            L = {k: f32(v) for k, v in c['live'].items()}
            exp += gfinal.render(c['glyph'], c['value'], L or None, state=c['state']).info['layers']
            st = -1 if c['state'] is None else c['state']
            flags = int(bool(L.get('tape_active'))) | int(bool(L.get('playing'))) << 1
            vals = ' '.join('%.9g' % L.get(k, 0) for k in ('amp', 'env', 'filter_env', 'phase', 'progress', 'mod', 'tape_phase', 'clock_phase'))
            lines.append(f"{ids.index(c['glyph'])} {c['glyph']} {c['case']} {c['value']} {st} {int(bool(L.get('active')))} {flags} {vals}")
        (td / 'cases.txt').write_text('\n'.join(lines) + '\n'); (td / 'expected.bin').write_bytes(bytes(exp))
        exe = td / 'gfinal_parity'
        cc = os.environ.get('CC', 'cc')
        subprocess.run([cc, '-O1', '-ffp-contract=off', '-w', '-DX0X_HOST', '-I', str(ROOT / 'firmware' / 'src'),
                        '-I', str(ROOT / 'firmware' / 'src' / 'app'), str(ROOT / 'tests' / 'host' / 'gfinal_parity.c'), '-o', str(exe)], check=True)
        r = subprocess.run([str(exe), str(td / 'cases.txt'), str(td / 'expected.bin')], capture_output=True, text=True)
        print(r.stdout, end=''); print(r.stderr, end='', file=sys.stderr)
    if bad_ref: print(f'FAIL: {bad_ref} reference frames drifted from reference-hashes.json')
    return 1 if bad_ref or r.returncode else 0

if __name__ == '__main__':
    sys.exit(main())
