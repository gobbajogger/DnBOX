"""Apply exact ordered archived diffs to the two original-sound schema files."""
from pathlib import Path
from collections import defaultdict
from hashlib import sha256
import json,re,sys

root=Path(sys.argv[1])
starter=Path(sys.argv[2])
out=Path(sys.argv[3])
routes=json.loads(Path(sys.argv[4]).read_text())
patches=list(root.rglob("*.patch"))

def hunks(p, name):
    select=False
    result=[]
    for line in p.read_bytes().splitlines(keepends=True):
        if line.startswith(b"--- "): select=False
        elif line.startswith(b"+++ "):
            tag=line[4:].split(None,1)[0].decode()
            select=(tag == name or tag.endswith("/" + name))
        elif select and re.match(rb"^@@ -\d+",line):
            result.append([[],[]])
        elif select and result:
            if line[:1] in (b" ",b"-"): result[-1][0].append(line[1:])
            if line[:1] in (b" ",b"+"): result[-1][1].append(line[1:])
    if not result: raise ValueError((p,name,"no patch hunks"))
    return [(b"".join(a),b"".join(b)) for a,b in result]

def apply(data, hs):
    result=[]
    pos=0
    for before, after in hs:
        if not before:
            raise ValueError("unexpected nonanchored hunk")
        off=data.find(before,pos)
        if off<0 or (off and data[off-1]!=10):
            raise ValueError("historical hunk preimage mismatch")
        result.extend((data[pos:off],after))
        pos=off+len(before)
    result.append(data[pos:])
    return b"".join(result)

for ext,spec in routes.items():
    name=spec.get("path","firmware/src/app/original_sound."+ext)
    data=(starter/name).read_bytes()
    for token in spec["route"]:
        matches=[p for p in patches if p.as_posix().endswith("/"+token)]
        if len(matches)!=1:
            raise ValueError((token,len(matches)))
        data=apply(data,hunks(matches[0],name))
    digest=sha256(data).hexdigest()
    if digest != spec["sha256"]:
        raise ValueError((name,"postimage mismatch",digest))
    dest=out/name
    dest.parent.mkdir(parents=True,exist_ok=True)
    dest.write_bytes(data)
    print(digest,name)
