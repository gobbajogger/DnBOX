from hashlib import sha256
from pathlib import Path
import json,sys
old,new=map(Path,sys.argv[1:])
def hash(x): return sha256(x).hexdigest()
def load(directory,m):
    path=Path(next(x for x in (directory/(m+'-loaded.log')).read_text().splitlines() if x.startswith('/tmp/dnbox-loaded-replay-')))
    raw=(path/'disposable.bin').read_bytes()
    msha=next(x.split()[3] for x in (directory/('loaded.log' if directory==old else 'replay.log')).read_text().splitlines() if x.startswith(m+' 0 '))
    assert hash(raw)==msha,(hash(raw),msha)
    assert 'flash: 0 erases / programs' in (path/'run.log').read_text()
    own=json.loads((path/'owner.json').read_text())
    assert own['count']>=4 and all(x['bank']==2 and x['arp']==1 for x in own['owners'])
    return raw,(path/'scene.keys').read_bytes(),json.loads((path/'result.json').read_text())
out={}
for m in ('elite','pro'):
    a,s0,x=load(old,m);b,s1,y=load(new,m);assert s0==s1
    assert len(a)==len(b)==0x100000
    for n in (0,1,2,3,5,6):
        p=0x9d000+8192*n;assert a[p:p+8192]==b[p:p+8192]
    assert a[0xaf000:0xaf010]==b[0xaf000:0xaf010]
    assert x['skipped_cycles']==y['skipped_cycles']==0
    out[m]=dict(scene_sha256=hash(s0),old=x,trial=y,delta_us=y['worst_us']-x['worst_us'],delta_xruns=y['xruns']-x['xruns'])
print(json.dumps(out,indent=2))
