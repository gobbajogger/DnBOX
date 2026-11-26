"""Independent original-source and DnBOX factory-patched event-only audition."""
from concurrent.futures import ThreadPoolExecutor
import hashlib,json,os,struct,subprocess,sys
from pathlib import Path
here=Path(__file__).resolve().parent; w=here.parent
base=w/'recovered-sound/s1-room-parity'
pack=base/'aligned-commands.bin'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(pack)=='622050ebc18a4dc98461d68f0f56b76924868b64c98bb370153c98cf3e577b41'
identity=json.loads((base/'aligned-bank.json').read_text())
tune=json.loads((base/'common-project-tuning/index.json').read_text())
ids=[*map(int,sys.argv[1:])]
known=set(json.loads((here/'interval-results.json').read_text()))
private=here/'private'
is_packed=os.environ.get('S1_TEST_REAL_PACK')=='1'
factory=private/('emit-packed' if is_packed else 'emit-patched')
original_pack=w/'recovered-sound/s1-bank-replay/original-private/pack.private'
assert sha(original_pack)=='d5e003e878ab46d6051d6cff16be557fbdfde7fccaa9255e4217bb5ffb51fa43'
all_rows=[]
with pack.open('rb') as f:
 magic,num=struct.unpack('<2I',f.read(8));assert magic==0x53315043 and num==906
 for i in range(num):
    n,=struct.unpack('<I',f.read(4));v=dict(struct.iter_unpack('<If',f.read(8*n)));assert n==144
    if (not ids and v[62]==1.) or i in ids:all_rows.append((i,v))
assert len(all_rows)==(len(ids) if ids else 393)
def run(*a,env):
 return subprocess.run(a,env=env,text=True,capture_output=True,check=True)
def first(a,b):
 for i in range(max(len(a),len(b))):
    x=a[i] if i<len(a) else None;y=b[i] if i<len(b) else None
    if x!=y:return dict(i=i,source=x,candidate=y)
def job(it):
 i,v=it
 d=private/('bank-events-packed' if is_packed else 'bank-events')/f'{i:03}'; d.mkdir(parents=True,exist_ok=True)
 npo=tune.get(str(i),{}).get('npo',12)
 env=dict(os.environ,S1_AUTHORED_TUNING_NPO=str(npo),S1_ARP_PACK=str(original_pack))
 state=d/'arp.state'
 state.write_text(''.join(f'{j} {float(v[j]).hex()}\n' for j in (*range(60,115),147)))
 ref=d/'upstream.events'; result=d/'factory.events'
 run(private/'orig-events',state,str(i),ref,str(npo),env=env)
 run(factory,state,str(i),result,'factory',env=env)
 real=(private/str(i)/'original.events')
 if str(i) in known:
    assert real.read_bytes()==ref.read_bytes(),(i,real,ref)
 a=ref.read_text().splitlines()[1:];b=result.read_text().splitlines()[1:]
 return (i,dict(label=identity['rows'][i]['label'],step=float(v[147]),bpm=float(v[64]),interval=float(v[61]),
    npo=npo,events_original=len(a),events_factory=len(b),first_mismatch=first(a,b),ref_sha256=sha(ref),factory_sha256=sha(result),full_graph_tape_checked=str(i) in known))
with ThreadPoolExecutor(max_workers=6) as pool:
 rows=dict(pool.map(job,all_rows))
out={'inputs':{'aligned_sha256':sha(pack),'factory_emitter_sha256':sha(factory),'original_sequencer_sha256':sha(private/'orig-events')},
     'real_pack_ingress':is_packed,'cases':len(rows),'exact':sum(v['first_mismatch'] is None for v in rows.values()),'full_graph_tape_checked':sum(v['full_graph_tape_checked'] for v in rows.values()),'rows':rows}
print(json.dumps(out,indent=2))
