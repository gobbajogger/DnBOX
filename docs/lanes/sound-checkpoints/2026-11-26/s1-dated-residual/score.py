import json,hashlib,math
from pathlib import Path
import numpy as np
from scipy.signal import butter,sosfiltfilt
F=44100; B=64; CELL=36; IDS=(62,151,191,366,459)
CAPS=(4534,22000,25792); STATE=128 # favourable total per-owner delay only
READ=1024; starts=[round(t*F)+128 for t in (1.,2.,3.,4.,4.5,5.,5.5)]
e=lambda x:float(np.sum(np.square(x,dtype=np.float64)))
db=lambda x:10*math.log10(max(float(x),1e-40))
def target(q,t):
 tt=(t.astype('f8')*(-float(F))).astype('f4')
 d=tt.astype('i4'); f=(tt-d.astype('f4')).astype('f4')
 cut=((q+d)<0)|(f<0)
 return np.stack((q+d-cut-1,q+d-cut,q+d-cut+1,q+d-cut+2)), (f+cut.astype('f4')).astype('f4')
def interp(x,idx,fr):
 f=fr.astype('f8'); z=(f*f-1)/6; y=(f+1)/2; w=y-1; a=3*z
 y-=a; w-=z; a-=f
 return (w*x[idx[0]]+a*x[idx[1]]+y*x[idx[2]]+z*x[idx[3]])*f+x[idx[1]]
def quant(v):
 m=float(np.max(np.abs(v))); s=float(np.float16(m/7))
 return np.clip(np.rint(v/s),-7,7)*s if s>0 else np.zeros(len(v))
def snapshot(n,t,write,hi,idx):
 span=[]
 for j,m in enumerate((2.,2.,1.,1.)):
  cells=math.ceil(float(t)*m*F)+4
  span.append((j,max(0,n-cells),n,cells))
 groups=[]; all_e=0.; all_w=0.; ns=0
 for j,first,last,_ in span:
  for b in range(first//B,(last-1)//B+1):
   a,c=max(first,b*B),min(last,(b+1)*B)
   h=hi[a:c,j]; y=quant(h); en=e(h)
   groups.append((en,j,b,a,c,e(y-h),y))
   all_e+=en; all_w+=e(write[a:c,j]); ns+=c-a
 groups.sort(key=lambda g:(-g[0],g[1],g[2]))
 prefix=np.cumsum([g[0] for g in groups])
 need={str(x):int(np.searchsorted(prefix,all_e*x)+1) if all_e else 0 for x in (.90,.99,.999)}
 need_abs=int(np.searchsorted(prefix,max(0.,all_e-ns*10**(-80/10)))+1) if all_e>ns*10**(-80/10) else 0
 full_codec_error=sum(g[5] for g in groups)
 out=dict(full_nibble_payload_bytes=36*len(groups),full_nibble_error_dbfs=db(full_codec_error/max(1,ns)),frame=n,T=float(t),window_cells=sum(s[3] for s in span),authored_high_dbfs=db(all_e/max(1,ns)),fraction_high_of_raw_db=db(all_e/max(all_w,1e-40)),blocks=len(groups),required_ideal_blocks=need,ideal_blocks_for_minus80dBFS=need_abs,caps={})
 for dec in (8,16):
  low_bytes=sum(math.ceil(math.ceil(s[3]/dec)/2) for s in span)
  for cap in CAPS:
   spare=cap-low_bytes-STATE; k=max(0,min(len(groups),spare//CELL))
   d=dict(low_payload_bytes=low_bytes,high_entries=k,admissible_low=spare>=0,bytes_left=spare,density=k/max(1,len(groups)))
   top=groups[:k]
   missing=max(0.,all_e-sum(g[0] for g in top)); quant_e=sum(g[5] for g in top)
   d.update(ideal_unretained_high_dbfs=db(missing/max(1,ns)),nibble_total_high_error_dbfs=db((missing+quant_e)/max(1,ns)),ideal_energy_kept=max(0.,1.-missing/max(1e-40,all_e)))
   # Best retrospective dates, reference low and its feedback state exact.
   kept=np.zeros_like(hi)
   for _,j,_,a,c,_,y in top:kept[a:c,j]=y
   ys=[]; rs=[]
   for j,(ix,fr) in enumerate(idx):
    ys.append(interp(kept[:,j],ix,fr));rs.append(interp(hi[:,j],ix,fr))
   LR=lambda v: np.stack((v[0],v[2]+v[3]),axis=1)
   o,r=LR(ys),LR(rs);err=o-r
   d['stem_high_error_dbfs']=[db(np.mean(y*y)) for y in err.T]
   d['stem_high_exact_dbfs']=[db(np.mean(y*y)) for y in r.T]
   d['stem_high_null_dbr']=[db(e(y)/max(e(x),1e-40)) for x,y in zip(r.T,err.T)]
   out['caps'][f'd{dec}-b{cap}']=d
 return out
names={62:'Irresistible Forces Pad',151:'BB Kubrick Hall Drone',191:'Mushroom',366:'Deeper Space Pad',459:'Infinite Tension FX'}
for id in IDS:
 a=np.fromfile(f'{id}.write','<f4').reshape(-1,8); rec=np.fromfile(f'{id}.state','<f4').reshape(-1,8)
 assert a.shape==rec.shape==(264728,8) and np.isfinite(rec).all()
 assert np.array_equal(rec[:,4],a[:,4]) and np.array_equal(rec[:,6]+rec[:,7],a[:,7])
 write=rec[:,:4].astype('f8'); out={}
 for fc in (2756.25,5512.5):
  # Noncausal high-pass is an analysis convenience, not a target kernel.
  hi=sosfiltfilt(butter(8,fc,'highpass',fs=F,output='sos'),write,axis=0)
  rows=[]
  for n in starts:
   t0=a[n,0]; q=np.arange(n,n+READ,dtype='i4'); t=a[q,0]
   idx=[target(q,t*np.float32(k)) for k in (2.,2.,1.,1.)]
   assert max(x[0].max() for x in idx)<n
   if min(x[0].min() for x in idx)<0:continue
   # No sampled address may be outside its advertised age horizon.
   for (ix,f),m in zip(idx,(2.,2.,1.,1.)):
    assert ix.min()>=max(0,n-math.ceil(float(t0)*m*F)-4)
   rows.append(snapshot(n,t0,write,hi,idx))
  out[str(fc)]=rows
 dN=Path(f'{id}.state')
 print(json.dumps(dict(id=id,name=names[id],capture_sha256=hashlib.sha256(Path(f'{id}.write').read_bytes()).hexdigest(),reconstructed_state_sha256=hashlib.sha256(dN.read_bytes()).hexdigest(),tail_T_and_feedback=a[-1,:2].tolist(),bands=out)),flush=True)
