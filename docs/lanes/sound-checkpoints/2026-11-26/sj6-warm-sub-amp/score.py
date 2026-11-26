from pathlib import Path
import ctypes as c, hashlib, io, json, math, re, sys, tarfile, zipfile,struct
import numpy as np
o,b=map(Path,sys.argv[1:])
# Reuse authorized matching sample capsule decoder, no compiled app import.
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('ipa=')[1].split('nat=Path(')[0]
exec(compile('ipa='+s,'existing private SFZ container decoder','exec'))
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(data)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(arc) as t:
 def take(a):
  p=f'./program-2-{a}/'
  m=json.load(t.extractfile(p+'capture.json'))
  r=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False).copy()
  assert sha(r.tobytes())==m['pcm_sha256'] and m['active']==m['after'] and m['frames']==r.shape[1]==148684
  return r,m
 refs=dict((a,take(a)) for a in ('authored','dry','authored-repeat'))
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
def cfgpin(a):
 cfg=b/f'sub-range.private-output/2-{a}.private.txt'
 lines=cfg.read_text().splitlines();nw=int(lines[1]); words=dict((int(x),float(y)) for x,y in map(str.split,lines[2:2+nw]))
 assert all(words[k]==0 for k in (32,33,35,201)) and words[202]>0
 return cfg,lines,nw,words
def run(a,enabled,mute=False,shape=0):
 cfg,lines,nw,words=cfgpin(a)
 if mute:
  words[202]=0.
  cfg=o/f'{a}-muted.cfg'
  cfg.write_text('\n'.join(lines[:2]+[f'{k} {v:.9g}' for k,v in words.items()]+lines[2+nw:])+'\n')
 name=f'{a}-{int(mute)}-{enabled}-{shape}'
 p=o/f'{name}-{len(list(o.glob("*.so")))}.so';p.write_bytes((o/'center.so').read_bytes())
 lib=c.CDLL(str(p))
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 lib.set_zone.argtypes=[c.c_void_p]+[c.c_uint]*5+[c.c_double]
 for g in regions:
    sr,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
    K,L,H,I,J=(int(g[x]) for x in ('pitch_keycenter','lokey','hikey','loop_start','loop_end'))
    lib.set_zone(med.ctypes.data,len(med),I,J,L,H,sr/(220.*2**((K-69)/12.)))
 lib.set_arm(enabled)
 lib.set_shape(shape)
 f=o/f'{name}.f32'
 argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0
 counts={k:c.c_uint.in_dll(lib,k).value for k in ('reads','loops','resets')}
 assert counts['loops']==0
 return np.fromfile(f,'<f4').reshape(-1,2).T.copy(),counts

# Record reached source preparation and per-sample sample-advance boundaries
# on the current pinned ARM64 owner. Its physical AMP words are projected,
# not proof that the installed AU routes every public word into this lane.
for a in ('dry',):
 with open(o/'compact.trace','w') as f:
  saved=__import__('os').dup(2)
  __import__('os').dup2(f.fileno(),2)
  try:x,cnt=run(a,1)
  finally:
   __import__('os').dup2(saved,2);__import__('os').close(saved)
 assert cnt=={'reads':123675,'loops':0,'resets':4}
compact=[]
for l in (o/'compact.trace').read_text().splitlines():
 if not l.startswith('SUBREL '):continue
 _,at,slot,gate,stage,amp,step,pos=l.split()
 compact.append((int(at),int(slot),int(gate),int(stage),*(float.fromhex(q) for q in (amp,step,pos))))
assert len(compact)==len(set(q[0] for q in compact))
compact_by={x[0]:x for x in compact}
from sender import observe
def original():
 from unicorn import UC_HOOK_CODE
 import struct
 s=Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0]
 env={};exec(compile(s,'verified modern sampler','exec'),env)
 Emu,SamplerDSP,BIN,cfg=[env[k] for k in ('Emu','SamplerDSP','BIN','cfg')]
 e=Emu(str(BIN)); d=SamplerDSP(e,44100)
 e.call(0x1003898bc,d.dsp,1)
 for g in regions:
    sr,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
    K,L,H,I,J=(int(g[x]) for x in ('pitch_keycenter','lokey','hikey','loop_start','loop_end'))
    d.load_sample(dict(note=K, freq=440*2**((K-69)/12),lokey=L,hikey=H,loop=True,
                       loop_start=float(I),loop_end=float(J),start=0.,rate=sr),med.astype('f4')/32768.)
 d.build_keymap()
 # Own constructor/timing, not rendered-app audio. Use cold Base defaults,
 # link the previously evidenced author→R law, no other preset-dependent edits.
 for k,v in cfg['sampler_control_writes']['sampler1'][:44]:d.set_param(k,v,True)
 _,_,_,raw=cfgpin('dry')
 f32=lambda x:float(np.float32(x))
 mapval=lambda x,lo,hi,p:f32(lo+(hi-lo)*float(f32(x))**p)
 A,D,S,R=(mapval(raw[36],.002,16,3.33),mapval(raw[37],.002,24,3.35),
          mapval(raw[38],0,1,2.5),mapval(raw[39],.002,24,3.35))
 sender=observe(env,raw[39])
 assert sender[-1]['release_physical_word']==R
 controls={0:f32(.808*raw[202]),1:0,2:0,3:0,7:0,8:0,9:0,10:A,11:0,12:D,13:S,14:0,15:R,
           20:1,25:0,28:0,29:0,32:1,36:20000,37:440,38:1,39:0,40:0,41:0,42:0,43:0,44:0,51:1}
 for k,v in controls.items():d.set_param(k,v,True)
 now=0; active_n=0; actual=[];prep=[];gates=[]
 def snap(*_):
  v=e.x(0);st=e.r32(v+0x340)
  if 38370<=now<39458 and st:
   prep.append((now,st,e.rf(v+0x5e4),struct.unpack('<d',e.rd(v+0x10,8))[0]))
 def tap(*_):
  nonlocal active_n
  v=e.x(24);at=now+active_n;active_n+=1
  if 38370<=at<39458:
    cursor,ratio,drive=struct.unpack('<3d',e.rd(v+0x10,24))
    actual.append((at,e.r32(v+0x340),e.rf(v+0x5e4),cursor,ratio,drive,e.rd(v+8,1)[0],e.s(12)))
 e.uc.hook_add(UC_HOOK_CODE,snap,begin=0x1003b4c54,end=0x1003b4c54)
 e.uc.hook_add(UC_HOOK_CODE,tap,begin=0x1003b7d40,end=0x1003b7d40)
 cue=refs['dry'][1]
 events={q['frame']:q for q in cue['schedule'] if q['frame']<39458}
 samples=[]
 for n in cue['blocks']:
  if now in events:
   q=events[now]; k=q['note']-12; on=q['action']=='on'
   assert k==36
   e.call(0x100389c84 if on else 0x100389c8c,d.dsp,k,86 if on else 0,floats=(0.,))
   gates.append((now,k,86 if on else 0))
  active_n=0
  samples.append(d.render(n)[0].copy())
  assert active_n<=n
  now+=n
  if now>=39458:break
 assert not e.unknown and len(gates)==2 and actual,(e.unknown,gates,len(actual),len(prep),float(np.max(np.abs(np.concatenate(samples)))))
 return actual,prep,np.concatenate(samples),gates,controls,env['pin'](BIN),sender
orig,pp,pcm,ev,control,binary,sender=original()
base=json.load(open('sj6-j60-warm-sub/result.json'))
# On/off instrumentation must preserve the already-scored carrier-on terminal.
for k in ('authored','dry','authored-repeat'):
 assert refs[k][1]['pcm_sha256']==base['real_app_ref_sha'][k]
ob={x[0]:x for x in orig};start=38434
marks=sorted(set([start-2,start-1,start,start+1,start+15,start+16,start+128,start+256,start+512]+
                [max(ob),max(ob)+1,max(compact_by),max(compact_by)+1]))
def pick(i):
 return dict(frame=i,orig=ob.get(i),compact=compact_by.get(i))

def run_trace(a,z,mute=False):
 import os
 with open(o/f'trace-{a}-{z}-{int(mute)}','w') as f:
  save=os.dup(2);os.dup2(f.fileno(),2)
  try:value=run(a,1,mute=mute,shape=z)
  finally:os.dup2(save,2);os.close(save)
 return value
arms={a:[run_trace(a,sh) for sh in (0,1)] for a in ('authored','dry')}
base=json.load(open('sj6-j60-warm-sub/result.json'))
center=json.load(open('sj6-j60-warm-center/result.json'))
for a in ('authored','dry'):
 assert arms[a][0][1]==arms[a][1][1] and np.array_equal(arms[a][0][0][:,:38434],arms[a][1][0][:,:38434])
 v=arms[a][0][0].astype('f8');y=refs[a][0].astype('f8')
 def rms(q):return np.sqrt(np.mean(q*q))
 assert abs(float(20*np.log10(rms(v[:,16384:]-y[:,16384:])/rms(y[:,16384:]))) -base['windows']['full'][a][1]['signed_dbr'])<1e-9
p0,c0=run_trace('authored',0,True);p1,c1=run_trace('authored',1,True)
assert c0['reads']==c1['reads']==0 and np.array_equal(p0,p1)
shadow={}
for l in (o/'trace-dry-1-0').read_text().splitlines():
 if l.startswith('SHAMP '):
  _,f,k,word,base=l.split()
  f=int(f);assert f not in shadow
  shadow[f]=float.fromhex(word)
oa=np.array([x[2] for x in orig if x[0]>=38434],dtype='f8')
sa=np.array([shadow[x[0]] for x in orig if x[0]>=38434],dtype='f8')
err=float(20*np.log10(np.linalg.norm(sa-oa)/np.linalg.norm(oa))) if np.any(sa!=oa) else None
assert np.flatnonzero(oa)[-1]==np.flatnonzero(sa)[-1]
def rms(a):return float(np.sqrt(np.mean(a*a)))
def db(a,b):return float(20*np.log10(a/b))
rows={}
for w,l,r in [('held',20794,38434),('first-release',38434,49459),('chord',53869,82534),('final-release',82534,104584),('full',16384,148684)]:
 row={}
 for a in ('authored','dry'):
  ref=refs[a][0][:,l:r].astype('f8')
  row[a]=[{'signed_dbr':db(rms(pc[:,l:r]-ref),rms(ref)),'level_db':db(rms(pc[:,l:r]),rms(ref))}
          for pc,count in arms[a]]
 rows[w]=row
print(json.dumps(dict(binary=binary,actual_amp_release_sender=sender,source_shape_model_error_dbr=err,shape_last_nonzero_frame=38434+int(np.flatnonzero(sa)[-1]),
   source_shape_exact_original_amp_words=int(np.count_nonzero(sa==oa)),source_shape_compact_source_owned_delta=max(x[0] for x in compact if x[4])-max(x[0] for x in orig if x[2]),release_shape_control_only=True,first_hold_samples_bit_exact=True,sub_mute_bit_exact=True,orig_and_compact_cursors=marks,windows=rows),indent=2,allow_nan=False))
