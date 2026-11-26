"""Compare actual hosted provider against independently executed current source."""
import json,struct
from model import model
src=json.load(open('fade.json'))['trials']
def bits(g):return struct.unpack('<I',struct.pack('<f',g))[0]
def f(v):return struct.unpack('<f',struct.pack('<f',v))[0]
def signature(t):
    edges,_,_=model(t)
    rate=t['rate'];step=f(1./max(1.,f(rate*f(.005))))
    requested={int(a):v for a,v in t['events'].items()}
    e=dict(edges);g=1.;req=0;mode=0;d=2166136261
    for i in range(1024):
        if i in requested:req=requested[i]
        if i in e:mode=e[i]
        g=max(0.,f(g-step)) if req!=mode else min(1.,f(g+step))
        d=((d^bits(g))*16777619)&0xffffffff
    return f'{d:08x}'
for a in open('host-1.txt').read().splitlines()[1:]:
    r,q,w,stress=map(int,a.split()[:4])
    t=src[(8 if r==48000 else 0)+2*w+q]
    chunks=a.split(' type ')[1].split()[:-1]
    assert chunks==[f'{i}:{m}' for i,m,_ in t['edges']]+[
        f"resets={len(t['resets_by_callback'])}",f'gain={signature(t)}'],(a,t)
assert open('host-0.txt').read().split()[-1]==open('host-1.txt').readline().split()[-1]
print('actual compact provider all mode edges, exact F32 fade words, real zero-resets and stress trace PASS')
