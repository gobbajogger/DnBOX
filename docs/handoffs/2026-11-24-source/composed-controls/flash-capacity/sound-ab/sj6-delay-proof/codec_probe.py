"""Offline first-echo storage experiment, not a realtime renderer.

Measure a bounded stereo IMA/DVI ADPCM ring with 128-frame restart blocks.
The resampler is zero-phase offline; causal conversion, live edits,
independent callback timing, and feedback are not qualified by this witness.
"""
import audioop
import json
import math
import struct
import numpy as np
from scipy.signal import resample_poly

RATE=44100
BUDGET=4096
BLOCK=128
START=struct.Struct('<hB')
BYTES=(START.size+BLOCK//2)*2
CAP=(BUDGET//BYTES)*BLOCK

def encode(chan):
    chan=np.asarray(np.clip(np.rint(chan*32767),-32768,32767),dtype='<i2')
    state=(0,0)
    out=[]
    for i in range(0,len(chan),BLOCK):
        seg=chan[i:i+BLOCK]; seg=np.pad(seg,(0,BLOCK-len(seg)))
        start=state
        code,state=audioop.lin2adpcm(seg.tobytes(),2,state)
        out.append(START.pack(*start)+code)
    return out

def decode(blocks,n):
    return np.concatenate([np.frombuffer(audioop.adpcm2lin(
        b[START.size:],2,START.unpack_from(b))[0],'<i2')
        for b in blocks])[:n].astype(float)/32767

def db(a,b):return round(20*math.log10(np.linalg.norm(a)/np.linalg.norm(b)),3)

def main():
    frames=2400
    t=np.arange(frames)/RATE
    def tone(hz):return .4*np.sin(2*np.pi*hz*t)*np.hanning(frames)
    def chord():
        weights=[(220,.2),(330,.16),(523.25,.12),(880,.085),(1320,.07),
                 (1680,.06),(2240,.05),(3000,.05),(5046,.04)]
        return np.hanning(frames)*sum(a*np.sin(2*np.pi*f*t) for f,a in weights)
    cases=[]
    for seconds in (.023622,.496063,1.003937):
        needed=math.ceil(seconds*RATE)
        # Leave room for the current restart block and two interpolation taps.
        stride=math.ceil(needed/(CAP-BLOCK-2))
        if ((BUDGET//BYTES)*BYTES)>BUDGET:raise AssertionError('ring size')
        for name,x in (('220 Hz',tone(220)),('3 kHz',tone(3000)),('bright chord',chord())):
            # Down/upsampling filters are achievably causal with lookahead or
            # phase compensation, but this witness charges neither state nor cost.
            low=resample_poly(x,1,stride)
            decoded=decode(encode(low),len(low))
            y=resample_poly(decoded,stride,1)[:len(x)]
            cases.append(dict(seconds=seconds,input=name,stride=stride,
                    rate_hz=round(RATE/stride,3),level_delta_db=db(y,x),
                    residual_db=db(y-x,x)))
    print(json.dumps(dict(bytes=BUDGET,encoded_bytes=(BUDGET//BYTES)*BYTES,frames=CAP,
        format='stereo IMA ADPCM 128-frame restart blocks; offline resampling',
        caveat='the 76-byte difference is not credited as sufficient native workspace',cases=cases),indent=2))
if __name__=='__main__':main()
