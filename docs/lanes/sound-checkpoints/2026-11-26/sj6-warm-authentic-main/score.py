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
def run(a,enabled,main=False,disable=None):
 cfg,lines,nw,words=cfgpin(a)
 if disable is not None:
  words[disable]=0.
  cfg=o/f'{a}-mute-{disable}.cfg'
  cfg.write_text('\n'.join(lines[:2]+[f'{k} {v:.9g}' for k,v in words.items()]+lines[2+nw:])+'\n')
 name=f'{a}-{disable}-{enabled}-{int(main)}'
 p=o/f'{name}.so';p.write_bytes((o/'center.so').read_bytes())
 lib=c.CDLL(str(p))
 lib.run_render.argtypes=[c.c_int,c.POINTER(c.c_char_p)]
 lib.set_zone.argtypes=[c.c_void_p]+[c.c_uint]*5+[c.c_double]
 for g in regions:
    sr,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
    K,L,H,I,J=(int(g[x]) for x in ('pitch_keycenter','lokey','hikey','loop_start','loop_end'))
    lib.set_zone(med.ctypes.data,len(med),I,J,L,H,sr/(220.*2**((K-69)/12.)))
 lib.set_arm(enabled)
 lib.set_main.argtypes=[c.c_void_p,c.c_void_p,c.c_uint,c.c_uint]
 lib.set_main(reference_main.ctypes.data,valid_main.ctypes.data,reference_main.shape[1],int(main))
 f=o/f'{name}.f32'
 argv=(c.c_char_p*3)(b'probe',str(cfg).encode(),str(f).encode())
 assert lib.run_render(3,argv)==0
 counts={k:c.c_uint.in_dll(lib,k).value for k in ('reads','loops','resets')}
 assert counts['loops']==0
 nn=(c.c_uint*8)();vv=(c.c_double*8)()
 lib.main_readback(nn,vv)
 counts['main_all_used_per_key']=list(nn[:4]);counts['main_source_present_per_key']=list(nn[4:])
 counts['main_stock_vs_actual_preOTA_dbr_by_key']=[float(10*np.log10(e/r)) if r and e else None for e,r in zip(vv[4:],vv[:4])]
 return np.fromfile(f,'<f4').reshape(-1,2).T.copy(),counts

from source_main import render
m=refs['dry'][1]
raw={int(v['address']):v['value'] for v in m['active']}
_,_,reference_main,valid_main,keys,controls,binary=render(m,raw)
valid_main=np.ascontiguousarray(valid_main,dtype='u1')
base=json.load(open('sj6-j60-warm-sub/result.json'))
wired=json.load(open('sj6-j60-warm-center/result.json'))
arms={a:[run(a,1,main=j) for j in (0,1)] for a in ('authored','dry')}
isolated_dry=[run('dry',0,main=j,disable=202) for j in (0,1)]
mute=[run('authored',1,main=j,disable=1) for j in (0,1)]
assert np.array_equal(mute[0][0],mute[1][0]) and all(not any(q['main_all_used_per_key']) for _,q in mute)
for a in ('dry','authored'):
 assert refs[a][1]['pcm_sha256']==base['real_app_ref_sha'][a]
 assert sha(cfgpin(a)[0].read_bytes()) in base['controls_compact_fixture_sha']
 ref=refs[a][0].astype('f8')
 y=arms[a][0][0].astype('f8')
 def rms(q):return np.sqrt(np.mean(q*q))
 assert abs(float(20*np.log10(rms(y[:,16384:]-ref[:,16384:])/rms(ref[:,16384:]))) -base['windows']['full'][a][1]['signed_dbr'])<1e-9
 assert arms[a][0][1]==arms[a][1][1] and arms[a][0][1]['reads']==123675 and arms[a][0][1]['resets']==4
 assert arms[a][0][1]['main_source_present_per_key']==list(map(int,valid_main.sum(axis=1)))
 assert np.array_equal(arms[a][0][0][:,:16384],arms[a][1][0][:,:16384])
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
def db(a,b):return float(20*np.log10(a/b)) if a and b else None
windows={}
for w,l,r in [('attack',16384,20794),('held',20794,38434),('release',38434,49459),('chord',53869,82534),('tail',82534,104584),('full',16384,148684)]:
 row={}
 for a in ('authored','dry'):
  y=refs[a][0][:,l:r].astype('f8')
  row[a]=[{'signed_dbr':db(rms(x[:,l:r]-y),rms(y)),'level_db':db(rms(x[:,l:r]),rms(y))}
          for x,_ in arms[a]]
 y,z=[pc[:,l:r].astype('f8') for pc,_ in isolated_dry]
 row['compact_main_only_boundary']={'same_donor_source_substitution_delta_dbr':db(rms(z-y),rms(y)),'level_delta_db':db(rms(z),rms(y))}
 windows[w]=row
assert all(q['reads']==0 for _,q in isolated_dry)
print(json.dumps(dict(binary=binary, source_controls=controls, source_keys=keys, source_preOTA_hash=sha(reference_main.tobytes()),
       raw_reference_main_from_generated_saw_actual_current_submitted_source=True, exactly_same_pinned_render_input_control_words=170,
       mapped_velocity_scope='authentic note receiver86, compact retains its existing same-cue MIDI ingress (nominal87), no new velocity correction',
       baseline_terminal_equal_prior=True,original_reference_sha=base['real_app_ref_sha'],
       guards={'main_muted_exact_hash':sha(mute[0][0].tobytes()),'quiet_before_note_exact':True},
       full_read_counts={a:[q for _,q in arm] for a,arm in arms.items()},
       isolate_main_scope='separate Main-only same compact terminal comparison. No actual installed Main-only native render on this exact schedule; no fabricated terminal score.',
       isolated_main_counts=[q for _,q in isolated_dry],windows=windows),indent=2,allow_nan=False))
