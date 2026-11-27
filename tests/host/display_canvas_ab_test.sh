#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Compare the ordinary UI through the same simulator controls on both builds.
# Clip/SMC snapshots and native audio/input service need separate tests.
set -eu
cd "$(dirname "$0")/../.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
X0X_HOST_OUT="$tmp/base" X0X_HOST_DEFS=-DX0X_UI_SMALL_CANVAS=0 sh host/build_host.sh >/dev/null
X0X_HOST_OUT="$tmp/trial" X0X_HOST_DEFS=-DX0X_UI_SMALL_CANVAS=1 sh host/build_host.sh >/dev/null
python3 - "$tmp" <<'PY'
from pathlib import Path
import sys
tmp = Path(sys.argv[1])
commands = ['wait 100']
count = 0

def shot(*actions):
    global count
    commands.extend((*actions, 'wait 50', f'shot {count:03d}.png'))
    count += 1

shot()
for button in ('HOME', 'SEL', 'FX', 'ENV', 'LFO', 'EDIT', 'SEQ', 'ARP', 'GLO', 'HOME'):
    shot(f'tap {button}')
    for enc in ('SELECT','ALGO','PRESET','K1','K2','K3','K4'):
        shot(f'turn {enc} 1 80')
    for key in (2, 7, 14, 18):
        shot(f'tapkey {key}')
shot('tap PLAY')
for _ in range(16):
    shot('wait 50')
shot('tap REC','tapkey 7')
(tmp / 'routes.txt').write_text('\n'.join(commands) + '\n')
PY
# Neither route should render a wall-clock audio-load number into the oracle.
mkdir "$tmp/original" "$tmp/compacted"
X0X_HOST_LOAD=0 X0X_HOST_DISPLAY_LOAD=0 "$tmp/base" "$tmp/routes.txt" "$tmp/original" >/dev/null
X0X_HOST_LOAD=0 X0X_HOST_DISPLAY_LOAD=0 "$tmp/trial" "$tmp/routes.txt" "$tmp/compacted" >/dev/null
python3 - "$tmp" <<'PY'
import hashlib
from pathlib import Path
import sys
tmp = Path(sys.argv[1])
a, b = tmp / 'original', tmp / 'compacted'
frames = sorted(a.glob('*.png'))
assert len(frames) == len(list(b.glob('*.png'))) == 139
hashes = set()
for p in frames:
    data = p.read_bytes()
    assert data == (b / p.name).read_bytes(), f'Different pixels: {p.name}'
    hashes.add(hashlib.sha256(data).digest())
assert len(hashes) >= 60, 'Insufficient distinct screens'
print(f'PASS: {len(frames)} display frames byte-identical, {len(hashes)} unique')
PY
