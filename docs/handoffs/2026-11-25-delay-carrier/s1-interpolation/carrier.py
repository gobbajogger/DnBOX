import numpy as np
sr,dd=44100,45
n=np.arange(sr*2)
idx,u=n//dd,(n%dd)/dd
amps=lambda x, hz, fs: 20*np.log10(max(1e-15,2*abs(np.dot(x,np.exp(-2j*np.pi*hz*np.arange(x.size)/fs)))/x.size))
shapes={'stock':u, 'cubic':u*u*(3-2*u), 'cosine':0.5-0.5*np.cos(np.pi*u)}
for src in (300,700,1300,3000):
 alias=abs(src-round(src/(sr/dd))*(sr/dd))
 x=np.cos(2*np.pi*src*n/sr)
 cell=x.reshape(-1,dd).mean(axis=1)
 print('input',src,'Hz; stored',alias,'Hz at',round(amps(cell,alias,sr/dd),3),'dB relative input')
 for name,w in shapes.items():
  out=cell[idx]*(1-w)+cell[(idx+1)%cell.size]*w
  print(f' {name} output alias and carrier/image: {amps(out,alias,sr):.3f}, {amps(out,src,sr):.3f} dB')
