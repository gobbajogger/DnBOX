#!/usr/bin/env python3
"""Give each replay arm the same explicit project A4 for nondefault 12ET rows."""
from pathlib import Path
import json,sys,math,hashlib,struct,shutil
manifest, identity, tuned, out=map(Path,sys.argv[1:])
shutil.copytree(tuned,out)
ix=json.loads((out/'index.json').read_text())
rows=json.loads(manifest.read_text())['cases']
bank=json.loads(identity.read_text())['rows']
for i,info in enumerate(bank):
    r=rows[info['original_order']]
    a4=float(math.trunc(dict(r['commands'])[143]))
    # A captured authored table already fixes frequencies in both arms.
    if a4==440 or str(i) in ix:continue
    assert 410<=a4<=470
    raw=struct.pack('<127f',*(min(22050.,a4*math.exp2((n-69)/12.)) for n in range(127)))
    fn=f'{i:03d}.f32'
    (out/fn).write_bytes(raw)
    ix[str(i)]={'file':fn,'label':info['label'],'npo':12,'reference_A4_hz':a4,'sha256':hashlib.sha256(raw).hexdigest()}
assert len(ix)==23
(out/'index.json').write_text(json.dumps(ix,indent=2)+'\n')
