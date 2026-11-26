"""Direct full-capture comparison, never aligns/skips a frame."""
import hashlib, json, struct, sys
from pathlib import Path
d=Path(__file__).resolve().parent
old, new=map(Path,sys.argv[1:])
def pcm(root,m):
    p=next(line for line in (root/f'{m}.log').read_text().splitlines() if 'dnbox-loaded-replay-' in line)
    return (Path(p)/'audio.s16le').read_bytes()
out=[]
for model in ('elite','pro'):
    a,b=(pcm(p,model) for p in (old,new))
    assert len(a)%4==len(b)%4==0
    ix=next((i for i in range(0,min(len(a),len(b)),4) if a[i:i+4]!=b[i:i+4]),None)
    out.append(dict(model=model, frame_count=[len(a)//4,len(b)//4], first_mismatched_common_frame=None if ix is None else ix//4,
                    direct_sha256=[hashlib.sha256(z).hexdigest() for z in [a,b]],
                    first_samples=None if ix is None else [struct.unpack_from('<2h',z,ix) for z in [a,b]]))
print(json.dumps(out,indent=2))
