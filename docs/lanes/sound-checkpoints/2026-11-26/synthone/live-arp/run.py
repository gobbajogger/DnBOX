import hashlib,json,os,subprocess,sys,concurrent.futures
from pathlib import Path
import numpy as np
s=Path(__file__).resolve().parent; root=s.parent
a=root/'recovered-sound/s1-room-parity'
f=a/'unified-releasephase'
reg=json.loads((a/'aligned-bank.json').read_text())
bank={x['id']:x for line in (a/'bank-releasephase.jsonl').read_text().splitlines()[1:] if 'ref_sha256' in (x:=json.loads(line))}
tuned=json.loads((a/'common-project-tuning/index.json').read_text())
ctx=Path(root/'s1-fx-parity/reference-build/factory-inputs/public-morphs.f32')
pack=root/'recovered-sound/s1-bank-replay/original-private/pack.private'
cmd=a/'aligned-commands.bin'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
for p,h in json.loads((a/'bank-releasephase.jsonl').read_text().splitlines()[0])['inputs'].items(): assert sha(root/p)==h
def tape(p):
    h,*ls=p.read_text().splitlines()
    return h,[list(map(int,x.split())) for x in ls]
def first(a,b):
 for i in range(max(len(a),len(b))):
    x=a[i] if i<len(a) else None;y=b[i] if i<len(b) else None
    if x!=y:return dict(event_index=i,source=x,dnbox=y)
def metrics(a,b):
    power=lambda x:np.mean(np.square(x),axis=0)
    return (10*np.log10(np.maximum(power(a-b),1e-40)/np.maximum(power(a),1e-40))).tolist()
def run(*args,env): 
    return subprocess.run(args,env=env,capture_output=True,text=True,check=True)
def job(i):
 d=s/'private'/str(i);d.mkdir(exist_ok=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('S1_')}
 env.update(S1_EXACT_MORPH_WAVES=str(ctx),S1_ORACLE_ARP_STATE=str(d/'authored.state'))
 ti=tuned.get(str(i))
 if ti:
    tf=a/'common-project-tuning'/ti['file'];assert sha(tf)==ti['sha256']
    env.update(S1_AUTHORED_TUNING_F32=str(tf),S1_AUTHORED_TUNING_NPO=str(ti['npo']))
 run(s/'private/original',ctx,cmd,str(i),d/'original.f32','wet',d/'original.events',env=env)
 assert sha(d/'original.f32')==bank[i]['ref_sha256']
 hh,ref=tape(d/'original.events')
 refpcm=np.fromfile(d/'original.f32',dtype='<f4').reshape(264600,2).astype('f8')
 modes={}
 for m in ('stock','project','panel','panel-source-interval','factory-packed','shared-positive'):
    if m=='shared-positive':infile=d/'original.events'; control='shared tape positive control only'
    elif m=='factory-packed':
      infile=d/(m+'.events')
      p=run(s/'private/emit-packed',d/'authored.state',str(i),infile,'factory',env=dict(env,S1_ARP_PACK=str(pack)));control=p.stderr.strip()
    else:
      infile=d/(m+'.events')
      p=run(s/'private/emit',d/'authored.state',str(i),infile,m,env=env);control=p.stderr.strip()
    run(f/'candidate',pack,str(i),d/(m+'.f32'),'wet',infile,d/(m+'.replay'),env=env)
    assert infile.read_bytes()==(d/(m+'.replay')).read_bytes()
    mh,me=tape(infile)
    if m in ('shared-positive','factory-packed'):assert sha(d/(m+'.f32'))==bank[i]['trial_sha256']
    trial=np.fromfile(d/(m+'.f32'),dtype='<f4').reshape(264600,2).astype('f8')
    modes[m]={'events':len(me),'first':first(ref,me),'tape_sha256':sha(infile),'state':control,'raw_wet_null_dbr':metrics(refpcm,trial)}
 return (i,{'name':reg['rows'][i]['label'],'original_events':len(ref),'original_event_sha256':sha(d/'original.events'),'modes':modes})
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as ex:
 print(json.dumps(dict(ex.map(job,map(int,sys.argv[1:]))),indent=2,allow_nan=False))
