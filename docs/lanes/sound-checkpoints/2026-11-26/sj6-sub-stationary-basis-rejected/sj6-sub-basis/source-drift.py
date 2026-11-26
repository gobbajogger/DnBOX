"""Observe authentic waveform first-harmonic phase over its existing raw clock.
Diagnostic only: no align, saved waveform/trace coefficients, or drift fit."""
from pathlib import Path
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('nat=Path(')[0]
exec(compile(s,'private verified media read-in only','exec'))
out={}
for sought in (36,54,73):
    region=next(r for r in regions if int(r['lokey'])<=sought<=int(r['hikey']))
    k=int(region['pitch_keycenter']); f=220*2**((k-69)/12)
    sr,med=pack[region['sample'].replace('\\','/').split('/')[-1]]
    n=sr//5;lo,hi=map(int,[region['loop_start'],region['loop_end']])
    w=np.hanning(n)
    anchor=lo+3*(hi-lo)//5
    def h(first,nr,w):
        x=med[first:first+nr].astype('f8')/32768.
        idx=first+np.arange(nr)
        return np.dot(x*w,np.exp((-2j*np.pi*f/sr)*idx))
    a=lo+3*(hi-lo)//5; b=lo+4*(hi-lo)//5
    href=h(a,b-a,np.hanning(b-a))
    rows=[]
    for tm in [0,.2,.4,.6,.8,1.,1.2,1.4,1.6,2.,2.4,3.,4.,5.]:
        i=round(tm*sr)
        if i+n>=len(med):break
        z=h(i,n,w)
        rows.append([tm, float(np.angle(z/href,deg=True))])
    out[sought]=dict(asset_root=k,nominal_f0=f,asset_rate=sr,training_sec=anchor/sr,absolute_phase_to_training_deg=rows)
print(json.dumps(out,indent=2))
