#!/usr/bin/env python3
"""Extract the original-preset tuning sidecar without changing any saved project."""
from pathlib import Path
import json,math,sys,struct
from hashlib import sha256
manifest, identity, assets, out=map(Path,sys.argv[1:])
out.mkdir(exist_ok=False)
ordered=json.loads(identity.read_text())["rows"]
rows=json.loads(manifest.read_text())["cases"]
banks={}
selected={}
for i,entry in enumerate(ordered):
    original=rows[entry["original_order"]]
    name=original["bank"]
    if name not in banks:banks[name]=json.loads((assets/name).read_text())
    preset=banks[name][original["preset_index"]]
    assert preset["name"]==entry["label"]==original["name"]
    assert str(preset["uid"]).lower()==str(original["uid"]).lower()
    values=preset.get("tuningMasterSet")
    if values is None:continue
    ratios=[]
    for x in values:
        assert math.isfinite(x) and x!=0
        x=abs(x)
        while x<1:x*=2.0
        while x>=2:x*=0.5
        ratios.append(x)
    ratios.sort()
    count=len(ratios)
    if count==12 and all(abs(x-2.0**(j/12))<=1e-6 for j,x in enumerate(ratios)):continue
    words=[]
    for n in range(127):
        f=(n-60)/count;o=math.trunc(f)
        if f<0:o-=1
        frac=abs(o-f)
        if frac==1:frac=0;o+=1
        k=math.floor(frac*count+.5)
        words.append(min(22050.,ratios[k]*2.0**o*261.6255653006))
    data=struct.pack("<127f",*words); file=f"{i:03d}.f32"
    (out/file).write_bytes(data)
    selected[i]={"file":file,"label":entry["label"],"npo":count,"sha256":sha256(data).hexdigest()}
assert len(selected)==16
(out/"index.json").write_text(json.dumps(selected,indent=2)+'\n')
