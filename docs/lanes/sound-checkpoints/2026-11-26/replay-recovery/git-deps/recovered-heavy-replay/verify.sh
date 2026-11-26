#!/bin/bash
set -euo pipefail
R=$(cd "$(dirname "$0")/.." && pwd)
cd "$R"
sha256sum -c - <<'SHAS'
fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017  restored-gate/fm1-live-ba83317a
01350ef744652ff7bc27501ddcd11037999c2f6f2bc8850ddaa2336ab6bd91cc  restored-harness-inspect/dot-gate-harness.zip
d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271  fw/experiments/sound-hardware-v1/strict-timing/heavy.keys
3cd2e16cc7119bf82e6274c3f46d2caad0dd576c0818dc43f9ab817a9dd1eee2  out/DnBOX-0.0.27-SMK37-elite-73749f55-test.fwsc
e62b1dfe4a03c32abc57660dd228fe40ce575d0dd1f2b7cf8920cce82029c785  out/DnBOX-0.0.27-SMK37-pro-73749f55-test.fwsc
3e78835b5c9928c68b4da092eb8c7677d2e37f07b47edc27c22ad795ae9d7c84  remote-typed-load/tree/build/smk37-elite/x0x.elf
SHAS
python3 - <<'PY'
from pathlib import Path
import hashlib,zipfile
scene=Path('fw/experiments/sound-hardware-v1/strict-timing/heavy.keys').read_text().strip().split(',')
expected=Path('restored-gate/fm1-live-ba83317a').read_bytes()
with zipfile.ZipFile('restored-harness-inspect/dot-gate-harness.zip') as z:
 assert z.read('fm1-live-ba83317a')==expected
assert len(scene)==922 and all(x.partition(':')[0].isdigit() and x.partition(':')[2] for x in scene)
print('922 events; archive runner matches byte-for-byte')
PY
python3 remote-typed-load/owner-arp-read.py remote-typed-load/tree/build/smk37-elite/x0x.elf remote-typed-load/loaded-smoke-elite/ram.bin
