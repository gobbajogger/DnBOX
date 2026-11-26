import hashlib,json,os,sys
from pathlib import Path
import numpy as np
here=Path(__file__).resolve().parent
base=here.parent
a=base/'recovered-sound/s1-room-parity'
identity=json.loads((a/'aligned-bank.json').read_text())
old={x['id']:x for line in (a/'bank-wet-aligned.jsonl').read_text().splitlines()[1:] if 'ref_sha256' in (x:=json.loads(line))}
kind=os.environ.get('S1_TAPE_DONOR','build5')
if kind in ('build-filter','build-morph'):
    pin={x['id']:x for line in (a/('filter-wet.jsonl' if kind=='build-filter' else 'morph-wet.jsonl')).read_text().splitlines()[1:] if 'ref_sha256' in (x:=json.loads(line))}
    p=here/('private/captures-filter' if kind=='build-filter' else 'private/captures-morph')
else:
    assert kind=='build5'
    pin=old
    p=here/'private/captures'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def m(ref,trial):
    powers=lambda x:np.mean(x*x,axis=0)
    db=lambda x,y:(10*np.log10(np.maximum(x,1e-40)/np.maximum(y,1e-40))).tolist()
    d={}
    for name,r in [('whole',slice(None)),('held',slice(0,198450)),('tail',slice(-44100,None))]:
        z,y=ref[r],trial[r]; d[name]={'null_dbr':db(powers(y-z),powers(z)),'rel_db':db(powers(y),powers(z))}
    return d
out={}
for i in [int(w) for w in sys.argv[1:]]:
    rows={}
    for mode in ('dry','wet'):
        stem=p/f'{i}-{mode}'
        paths={part:Path(f'{stem}.{part}') for part in ['original','direct','adapted','tape','replay']}
        assert paths['tape'].read_bytes()==paths['replay'].read_bytes()
        head,*lines=paths['tape'].read_text().splitlines()
        _,n,total,active,seq=head.split()
        assert int(n)==i and int(total)==264728 and (int(active) or not lines)
        events=[list(map(int,s.split())) for s in lines]
        assert all(0<=f<264728 and 0<=key<128 and 0<=vel<=127 for f,key,vel in events)
        assert all(after[0]>=before[0] for before,after in zip(events,events[1:]))
        full={k:np.fromfile(paths[k],dtype='<f4').astype('f8').reshape(-1,2) for k in ('original','adapted','direct')}
        assert all(y.shape==(264600,2) and np.isfinite(y).all() for y in full.values())
        hashes={k:sha(paths[k]) for k in paths}
        if not int(active):assert hashes['direct']==hashes['adapted']
        if mode=='wet':
            assert hashes['original']==old[i]['ref_sha256']
            if i in pin:assert hashes['direct']==pin[i]['trial_sha256']
        rows[mode]={'tape_active':bool(int(active)),'sequencer':float(seq),'events':len(events),
          'ons':sum(vel>0 for _,_,vel in events),'first_events':events[:4],
          'last_event':events[-1] if events else None,'original_direct':m(full['original'],full['direct']),
          'original_adapted':m(full['original'],full['adapted']),'hashes':hashes}
    assert rows['wet']['hashes']['tape']==rows['dry']['hashes']['tape']
    out[i]={'label':identity['rows'][i]['label'],**rows}
print(json.dumps(out,indent=2,allow_nan=False))
