"""Measure installed SuperJ6 harmonic spectra by key, no gain or time alignment."""
from pathlib import Path
import hashlib,json,sys
import numpy as np
r=Path(sys.argv[1]); read=lambda p:json.loads(p.read_text()); assert read(r/'result.json')['state']=='pass'
ent={}
for p in sorted(r.glob('native-jupiter-sine-gain-authored-band-*')):
 m=read(p/'capture.json'); shape=m['ablation_arm'].split('-')[1]; key=int(m['ablation_arm'].split('-')[-1])
 assert m['calibration_cue']=='jupiter-band-map-v1' and [(e['frame'],e['note']) for e in m['schedule']]==[(16384,key),(60484,key)]
 assert m['forced_controls']['83']==float(np.float32(1/3 if shape=='tri' else 0)) and m['forced_controls']['123']==0
 for id_,value in {'7':1,'8':0,'11':0,'17':0,'23':0,'32':1,'33':0,'40':0,'41':0,'59':0,'107':0,'129':1,'131':0,'205':1,'209':0}.items():
  assert m['forced_controls'][id_]==value
 x=np.load(p/'output.npy',allow_pickle=False)
 assert hashlib.sha256(x.tobytes()).hexdigest()==m['pcm_sha256']
 fc=440*2**((key-69)/12); nmax=min(24,int(.48*44100/fc)); start=25000; stop=55000
 nt=np.arange(1,nmax+1); ang=2*np.pi*fc/44100*np.outer(np.arange(start,stop,dtype=float)-16384,nt)
 G=np.concatenate([np.cos(ang),np.sin(ang),np.ones((stop-start,1))],axis=1)
 y=x[:,start:stop].T.astype(float); coef=np.linalg.lstsq(G,y,rcond=None)[0]
 h=coef[:nmax]-1j*coef[nmax:2*nmax]
 rati=h*np.exp(-1j*nt[:,None]*np.angle(h[:1]))/np.abs(h[:1])
 rem=y-np.einsum("ij,jk->ik",G,coef,optimize=False)
 xband=np.float32(64*fc/44100); row=0
 while xband>=np.float32(.49) and row<6:
  xband=np.float32(.5)*xband;row+=1
 blend=float(np.clip((xband-np.float32(.4))/np.float32(.09),0,1)) if row<6 else 0
 energy=float(np.square(abs(h[:,0])).sum())
 rec={'key':key,'shape':shape,'repeat':m['repeat'],'fundamental_hz':fc,'standalone_selector_row_blend':[row,blend],
      'h1_deg_from_sample_16384':np.angle(h[0],deg=True).tolist(),'h1_peak':np.abs(h[0]).tolist(),
      'harmonics_relative_db':(20*np.log10(np.maximum(1e-15,np.abs(rati)))).tolist(),
      'relative_phases_degrees':np.angle(rati,deg=True).tolist(),
      'harmonic_residual_dbr':(20*np.log10(np.linalg.norm(rem,axis=0)/np.linalg.norm(y,axis=0))).tolist(),
      'h_above_12_fraction_of_scored_energy':float(np.square(abs(h[12:,0])).sum()/energy),
      'h_above_18_fraction_of_scored_energy':float(np.square(abs(h[18:,0])).sum()/energy),
      'pcm_sha256':m['pcm_sha256']}
 ent[shape,key,m['repeat']]=rec
assert len(ent)==26
print(json.dumps([ent[k] for k in sorted(ent)],indent=2))
