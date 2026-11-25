import io,json,sys,wave,zipfile,hashlib
import numpy as np
from pathlib import Path
f=Path(sys.argv[1]);rows=[]
names={62:'Irresistible Forces Pad',113:'Dark Theater Organ',289:'JEC Atmostrings 2',435:'Red Minor'}
def power(x):return float(np.mean(np.square(x)))
def delta(a,b):return 10*np.log10(max(a,1e-30)/max(b,1e-30))
def bands(a,b,sr):
 n=a.shape[0];window=np.hanning(n)[:,None];freq=np.fft.rfftfreq(n,1/sr)
 ai=np.abs(np.fft.rfft(a*window,axis=0))**2
 bi=np.abs(np.fft.rfft(b*window,axis=0))**2
 return {f'{lo}-{hi}':round(delta(bi[(freq>=lo)&(freq<hi)].sum(),ai[(freq>=lo)&(freq<hi)].sum()),2) for lo,hi in [(30,250),(250,1000),(1000,4000),(4000,12000),(12000,20000)]}
with zipfile.ZipFile(f) as z:
 for k in sorted(z.namelist()):
  if '/listening/' not in k or not k.endswith('.wav'):continue
  raw=z.read(k)
  with wave.open(io.BytesIO(raw)) as w:
   assert w.getparams()[:4]==(2,2,48000,216000)
   a=np.frombuffer(w.readframes(216000),'<i2').reshape(-1,2).astype('f8')/32768
  p=int(Path(k).stem.split('-')[-1])
  a,sil,b=a[:96000],a[96000:120000],a[120000:]
  assert not np.any(sil)
  tail=slice(72000,None)
  r=dict(id=p,input_sha256=hashlib.sha256(raw).hexdigest(),known_name=names.get(p),original_peak=round(float(np.abs(a).max()),4),compact_peak=round(float(np.abs(b).max()),4),compact_minus_original_rms_db=round(delta(power(b),power(a)),2),last_500ms_compact_minus_original_rms_db=round(delta(power(b[tail]),power(a[tail])),2),last_500ms_band_energy_delta_db=bands(a[tail],b[tail],48000),raw_aligned_recorded_null_db_relative_original=round(delta(power(a-b),power(a)),2))
  rows.append(r)
print(json.dumps(dict(scope='Orchestrator 13:40 WIP archive; existing 2s listening PCM, not current firmware or exact full-tail acceptance',rows=rows),indent=2))
