import concurrent.futures as cf
import json,subprocess,sys
from pathlib import Path
d=Path(__file__).resolve().parent
r=d.parent/'s1-fx-parity/reference-build'
meta=json.loads((r/'commands/ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
def run(n):
    s=subprocess.check_output([d/'build/reference',r/'factory-inputs/public-morphs.f32',r/'commands/preset-commands.bin',str(n),'/dev/null','wet'],text=True).split()
    assert int(s[0])==n and len(s)==6
    return n,*map(float,s[1:])
with cf.ThreadPoolExecutor(max_workers=4) as exe: rows=list(exe.map(run,range(len(meta))))
active=[a for a in rows if a[1]>.5 and a[2]>0]
out={'source_rows':len(rows),'active_delay':len(active),'sync_active_delay':sum(a[5]>.5 for a in active),
     'active_target_at_most_seconds':{t:sum(a[3]<=t+1e-8 for a in active) for t in (.025,.05,.1,.1875,.25,.5,.66667,1.,2.,2.5)},
     'max_active_target':max(a[3] for a in active),
     'requested_named':{i:[meta[i]['name'],*rows[i][1:]] for i in (62,113,151,366)}}
print(json.dumps(out,indent=2))
