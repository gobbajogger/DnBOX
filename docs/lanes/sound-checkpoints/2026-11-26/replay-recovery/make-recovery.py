#!/usr/bin/env python3
"""Archive project-generated emulator inputs; do not include any original AU or app."""
from hashlib import sha256
from pathlib import Path
import json,shutil
here=Path(__file__).resolve().parent
root=here.parents[1]
stage=here/'files'
stage.mkdir(exist_ok=True)
def digest(f): return sha256(f.read_bytes()).hexdigest()
def copy(p,label=None):
    if not isinstance(p,Path): p=root/p
    dst=stage/(label or p.relative_to(root))
    dst.parent.mkdir(exist_ok=True,parents=True)
    if dst.exists(): assert dst.read_bytes()==p.read_bytes()
    else: shutil.copy2(p,dst)
    return dst
def findseed(m,sha):
    return next(x for x in sorted(Path('/tmp').glob(f'dnbox-emulator-only-*/{m}-widen-room.bin')) if digest(x)==sha)
# Generated/emulator-only inputs. These are not installable artifacts.
pins={'elite':'d9953d21980f6ae7dc26296c8b513618f11389db87beb54430ff7c2d3377ff31',
      'pro':'487e879c06aa19fc67ed809f6cbfa15fc1d80759b90326c2e0c3cc39b3271600'}
for m,sha in pins.items():
    copy(findseed(m,sha),f'sealed/{m}-h16-virtual-nor.bin')
    copy(f's1-organ-wet-recovery/native-widen-bfp/tree/widen-native/{m}/never-install.elf',f'sealed/{m}-h16-never-install.elf')
    copy(f'out/DnBOX-0.0.27-SMK37-{m}-73749f55-test.fwsc')
for p in ('restored-harness-inspect/dot-gate-harness.zip',
          'remote-typed-load/tree/build/smk37-elite/x0x.elf','remote-typed-load/loaded-smoke-elite/ram.bin'):
    copy(p)
# Exact project-owned source overlay relative to the named Git base; no object
# files, AU/app installers, keychain, PSRAM or staging environments.
d=root/'s1-organ-wet-recovery/native-widen-bfp/tree'
b=root/'fw'
exts={'.c','.h','.inc','.py','.S','.ld'}
overlay=[]
for folder in ("firmware","tools","experiments/original_synth","experiments/wobble","build/smk37-elite/gen","build/smk37-pro/gen"):
    for p in sorted((d/folder).rglob('*')):
        if p.suffix not in exts or not p.is_file(): continue
        rel=p.relative_to(d)
        old=b/rel
        if old.is_file() and old.read_bytes()==p.read_bytes() and not rel.parts[0]=='build': continue
        copy(p,Path('donor-overlay')/rel)
        overlay.append((str(rel),digest(p)))
index={'base_repo':'gobbajogger/DnBOX','base_sha':'73749f55d17a0643f1ddd072114172d29eecc2ae','source_overlay':overlay}
(stage/'SOURCE-OVERLAY.json').write_text(json.dumps(index,indent=2)+'\n')
