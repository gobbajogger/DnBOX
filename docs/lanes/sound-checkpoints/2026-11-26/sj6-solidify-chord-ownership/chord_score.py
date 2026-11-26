from pathlib import Path
import hashlib, io, json, sys, tarfile
import numpy as np
p=Path(sys.argv[1])
r=json.loads((p/'chord-param.json').read_text())
q=[x for x in r['presses'] if x['target']==35]
assert len(q)==2
def info(x):
 z={}
 for i,k,freq,_,inc,phase,stage,*other in x['ticks']:
  if i in (49459,49475,49587,49715,49843,49971,50227):
   z[i]={'received_key':k,'requested_freq':freq,'instant_hz':inc*44100,'phase':phase,'amp_stage':stage}
 return z
im,gl=map(info,q)
assert all(abs(q['instant_hz']-195.9977264)<.001 for i,q in im.items() if i>49459)
assert gl[49475]['instant_hz']<131 and gl[50227]['instant_hz']<138
assert im[49459]['phase']==gl[49459]['phase']==0
s=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha(s.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
real={}
with tarfile.open(s) as tf:
 for arm in ('dry','authored','authored-repeat'):
  root=f'./program-1-{arm}/'; c=json.load(tf.extractfile(root+'capture.json'))
  x=np.load(io.BytesIO(tf.extractfile(root+'output.npy').read()),allow_pickle=False)
  assert sha(x.tobytes())==c['pcm_sha256'] and c['active']==c['after'] and x.shape==(2,148684)
  y=x[0]; values={}
  for a in [49459,50227,51459,53459,55459]:
   spec=np.abs(np.fft.rfft(y[a:a+2048]*np.hanning(2048),262144))
   def peak(lo,hi):
    st,en=(int(f*262144/44100) for f in (lo,hi))
    return (st+int(np.argmax(spec[st:en])))*44100/262144
   values[a]=[peak(40,112),peak(126,210)]
  assert all(96<j[0]<100 and 194<j[1]<198 for j in values.values())
  real[arm]={'pcm_sha':c['pcm_sha256'],'dominant_ridges_hz':values,'prechord_rms':float(np.sqrt(np.mean(y[48435:49459]**2)))}
print(json.dumps(dict(receiver_no_glide=im,receiver_glide_on_nonfactory_control=gl,real_warm=real,scope='First 2048 frames prove no long receiver-slew transplant; no direct installed per-lane output. Delay shorter than first analysis window not excluded. No correction to note ownership.'),indent=2))
