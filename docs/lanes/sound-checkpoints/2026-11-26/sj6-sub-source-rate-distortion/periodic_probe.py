"""Lossless pitch-lag prediction of chosen quantized words, never synthesize media."""
import json
import math
from pathlib import Path
import sys
import zlib
import numpy as np
from source_probe import read
from predictor_probe import packed_plane

def signed_mod(x,bits):
    h=1<<(bits-1)
    return ((x+h)&((1<<bits)-1))-h

def encode(q,bits,lag,frac):
    h=lag+1
    prior=np.zeros_like(q)
    a=q[1:-lag]
    b=q[:-h]
    prior[h:]=a+(((b-a)*frac+128)>>8)
    u=signed_mod(q-prior,bits).astype('i4')
    return u

def decode(u,bits,lag,frac):
    h=lag+1
    r=np.zeros_like(u)
    # A vector of at most lag has no dependency on another value in that vector.
    for beg in range(0,len(u),lag):
        end=min(beg+lag,len(u))
        lo=max(h,beg)
        if lo>beg:
            r[beg:min(lo,end)]=u[beg:min(lo,end)]
        if end>lo:
            a=r[lo-lag:end-lag]
            b=r[lo-h:end-h]
            p=a+(((b-a)*frac+128)>>8)
            r[lo:end]=signed_mod(p+u[lo:end],bits)
    return r

def run(sfz,blob,scored):
    previous=json.loads(scored.read_text())['zones']
    zs={}
    for key,rate,raw,_,_ in read(sfz,blob):
        full=raw.astype('i4')
        z={}
        for bits in range(8,13):
            half=1<<(bits-1)
            q=np.clip(np.rint(full*half/32768),-half,half-1).astype('i4')
            period=rate/(220*2**((key-69)/12))
            lags=[(round(period),0),
                  (math.floor(period),round(256*(period-math.floor(period)))),
                  (math.floor(period*2),round(256*(period*2-math.floor(period*2))))]
            o={}
            for j,(lag,frac) in enumerate(lags):
                u=encode(q,bits,lag,frac)
                assert np.array_equal(q,decode(u,bits,lag,frac))
                zz=((u<<1)^(u>>31)).astype('i4')
                assert np.array_equal(u,(zz>>1)^(- (zz&1)))
                for lab,v in [('flat',u & ((1<<bits)-1)), ('zig',zz)]:
                    as_binary=v.astype('u1' if bits<=8 else '<u2').tobytes()
                    as_plane=packed_plane(v,bits)
                    for form, payload in [('word',as_binary),('plane',as_plane)]:
                        enc=zlib.compress(payload,6)
                        assert zlib.decompress(enc)==payload
                        o[f'{j}:{lab}:{form}']=len(enc)
            z[bits]=dict(payload=o,worst=previous[str(key)][f'1:{bits}']['worst_signed_dbr'])
        zs[key]=z
    summary={}
    for threshold in (-30,-40,-45,-50,-60):
        picks={}
        for k,z in zs.items():
            eligible=[(value,p,bits) for bits,x in z.items() if x['worst']<=threshold
                      for p,value in x['payload'].items()]
            if eligible:
                picks[k]=min(eligible)
        if len(picks)==len(zs):
            summary[threshold]=dict(payload_B=sum(v[0] for v in picks.values()), selections=picks)
    return dict(summary=summary,zones=zs,all_words_roundtrip_exact=True,
      scope='lossless long-pitch prediction after ordinary lossy fixed quantization; deflate sizes exclude anchors/index/decoder/cache; no audio exports or native measurement')
if __name__=='__main__':
    print(json.dumps(run(*map(Path,sys.argv[1:])),indent=2))
