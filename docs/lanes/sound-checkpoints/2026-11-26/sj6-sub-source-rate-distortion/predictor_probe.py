"""Measure reversible predictive size of already quantized private source."""
import json
import sys
from pathlib import Path
import zlib
import numpy as np
from scipy.signal import resample_poly
from source_probe import db, read, score

WINDOWS=('strike','body','loop-head','loop-tail','wrap-in','wrap-out','whole')
FORMATS=('flat','delta1','delta2','plane','plane-delta1','plane-delta2')
THRESHOLDS=(-30,-40,-45,-50,-60)

def packed_plane(u,bits):
    return b''.join(np.packbits((u>>i)&1).tobytes() for i in range(bits))
def unpack_plane(raw,n,bits):
    row=(n+7)//8
    u=np.zeros(n,dtype='i4')
    for i in range(bits):
        u|=np.unpackbits(np.frombuffer(raw[i*row:(i+1)*row],'u1'))[:n].astype('i4')<<i
    return u

def measure(w,bits):
    mask=(1<<bits)-1
    size={}
    for n in range(3):
        u=w&mask
        for _ in range(n):
            u=np.diff(u,prepend=0)&mask
        for planar in [False,True]:
            if planar:
                a=packed_plane(u,bits)
            else:
                a=u.astype('u1' if bits<=8 else '<u2').tobytes()
            enc=zlib.compress(a,6)
            back=zlib.decompress(enc)
            if planar:
                d=unpack_plane(back,len(u),bits)
            else:
                d=np.frombuffer(back,'u1' if bits<=8 else '<u2').astype('i4')
            assert np.array_equal(u,d)
            for _ in range(n):
                d=np.cumsum(d,dtype='int64').astype('i4')&mask
            assert np.array_equal(w&mask,d)
            label=('plane' if planar else 'flat') if not n else (('plane-' if planar else '')+f'delta{n}')
            size[label]=len(enc)
    return size

def run(sfz,bank):
    zones={}
    for key,rate,raw,lo,hi in read(sfz,bank):
        x=raw.astype('f8')/32768
        guard=round(10*rate/(220*2**((key-69)/12)))
        win=dict(zip(WINDOWS,[(0,rate//20),(rate//10,rate//2),(lo+guard,lo+2*guard),
            (hi-2*guard,hi-guard),(hi-256,hi),(lo,lo+256),(0,len(raw))]))
        z=zones.setdefault(key,{})
        for f in (1,2,3,4):
            a=x if f==1 else resample_poly(x,1,f,window=('kaiser',9.0),padtype='line')
            for bits in (8,9,10,11,12):
                s=2**(bits-1)
                q=np.clip(np.rint(a*s),-s,s-1).astype('i4')
                d=q/s
                if f!=1:
                    d=resample_poly(d,f,1,window=('kaiser',9.0),padtype='line')[:len(x)]
                met=score(d,x,win)
                z[f'{f}:{bits}']={'bits':bits,'worst_signed_dbr':max(v[0] for v in met.values()),
                                    'metrics':met,'bytes':measure(q,bits)}
    result={}
    for level in THRESHOLDS:
        cases={}
        for fmt in FORMATS:
            selected={}
            for key,z in zones.items():
                admissible=[(v['bytes'][fmt],label) for label,v in z.items() if v['worst_signed_dbr']<=level]
                if admissible:
                    selected[key]=min(admissible)
            if len(selected)==len(zones):
                cases[fmt]={'payload':sum(v[0] for v in selected.values()),'selection':selected}
        if cases:
            result[str(level)]=cases
    return {'thresholds_are_diagnostic_not_shipping':True,'all_decoder_values_roundtrip_exact':True,
        'summary':result,'zones':zones,'source_frames':sum(len(row[2]) for row in read(sfz,bank)),
        'scope':'one whole stream per zone, no index, random-access reset, boot cache, on-target decoder, or BREAK allocation included'}

if __name__=='__main__':
    print(json.dumps(run(*map(Path,sys.argv[1:])),indent=2,allow_nan=False))
