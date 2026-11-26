from pathlib import Path
import numpy as np,hashlib,json
s=Path('sj6-warm-postvoice-physical/score.py').read_text().split('from dose import collect')[0]
s=s.replace('o,b=map(Path,sys.argv[1:])','b=Path("sj6-cross-preset")')
ns={};exec(compile(s,'authenticated cached original cue/media held in memory','exec'),ns)
raw={int(v['address']):v['value'] for v in ns['refs']['dry'][1]['active']}
s=Path('sj6-warm-postvoice-physical/dose.py').read_text().replace("for name,id in (('Main',1),('Sub',202)):","for name,id in (('Sub',202),):")
for k,v in [('            v=e.x(24);key=e.r32(v+0x4c8)','\n            f=at+offsets[key];amp[keys.index(key),f]=e.rf(v+0x5e4)'),('        offsets={k:0 for k in keys}','\n        amp=np.zeros((4,cue["frames"]),dtype="f4")'),('obs[name]=(x,control,by)', 'REPLACE')]:
 assert s.count(k)==1
 s=s.replace(k,k+v if v!='REPLACE' else 'obs[name]=(x,control,by,amp)')
env={};exec(compile(s,'genuine compiled original Sub receiver','exec'),env)
source,binary=env['collect'](ns['refs']['dry'][1],raw,ns['regions'],ns['pack'])
out,control,by,amp=source['Sub']
known=json.load(open('sj6-warm-raw-postvoice/result-final.json'))
assert hashlib.sha256(out.tobytes()).hexdigest()==known['source_roles']['Sub']['output_sha']
j={}
for i,k in enumerate((36,48,52,55)):
 l,r,last,off,ct=by[k];q=amp[i,off:r+1]
 assert len(q)==560 and q[-1]==0 and q[-2]>0
 j[k]=dict(h=hashlib.sha256(q.tobytes()).hexdigest(),last=last,count=len(q))
print(json.dumps(dict(binary=binary,role_pcm_sha=hashlib.sha256(out.tobytes()).hexdigest(),notes=j),indent=2))
