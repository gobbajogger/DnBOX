"""Measure wet repeats and bounded host behavior; no device state or files."""
import argparse
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess
import tempfile
import time

import numpy as np
from scipy.signal import firwin
from sj6_stereo_delay import library
RATE=44100
DELAY=21876
N=220000
F=np.ctypeslib.ndpointer(np.float32,1,flags='C_CONTIGUOUS')
H=np.ctypeslib.ndpointer(np.int16,1,flags='C_CONTIGUOUS')
def load(folder, tmp, taps, stride):
    out=Path(tmp)/'stream.so'
    subprocess.run([os.environ.get('CC','cc'),'-std=c11','-O2','-Wall','-Wextra','-Werror',
         '-fPIC','-shared','-ffp-contract=off',f'-DPRE={taps}',f'-DSTRIDE={stride}',str(folder/'stream_probe.c'),'-lm','-o',str(out)],check=True)
    s=C.CDLL(str(out))
    s.stream_new.restype=C.c_void_p
    s.stream_free.argtypes=[C.c_void_p]
    s.stream_params.argtypes=[C.c_void_p,C.c_int,C.c_float,C.c_float]
    s.stream_process.argtypes=[C.c_void_p,C.c_size_t,F,F,F,F,H]
    s.stream_worst.argtypes=[C.c_void_p]
    for f in ['stream_state','stream_ring','stream_init_coeff_bytes','stream_fir_taps','stream_stride']:getattr(s,f).restype=C.c_size_t
    return s

def signal(name,n=None):
    if n is None:n=max(N,4*DELAY+3800)
    k=2400; t=np.arange(k)/RATE
    def tone(f):return .35*np.hanning(k)*np.sin(2*np.pi*f*t)
    x=np.zeros((2,n),dtype='f')
    if name=='chord':
        x[0,:k]=sum(tone(f)/w for w,f in enumerate((220,330,523,880,1320,1680,2240,3000,5050),2))
        x[1,730:730+k]=sum(tone(f)/w for w,f in enumerate((277,415,554,831,1245,1662,2217,3120,4700),2))
    else:x[0,:k]=tone(int(name))
    return x
def rms(x):return np.sqrt(np.mean(np.square(x,dtype=np.float64)))
def score(a,b):
    return dict(level_db=round(float(20*np.log10(rms(a)/rms(b))),3),
                residual_db=round(float(20*np.log10(rms(a-b)/rms(b))),3))
def render(s,x,coef,partitions=False,edited=False):
    y=np.empty_like(x)
    h=s.stream_new();assert h
    try:
        assert s.stream_params(h,DELAY,.62,.2)
        assert not s.stream_params(h,500000,.62,.2)
        i=0
        sizes=(17,128,251,327,512)
        k=0
        while i<x.shape[1]:
            if edited and i==30000:assert s.stream_params(h,DELAY-91,.62,.2)
            if edited and i==36000:assert s.stream_params(h,DELAY-4,.62,.2)
            end=min(x.shape[1],i+(sizes[k%len(sizes)] if partitions else N))
            if edited:
                for cut in (30000,36000):
                    if i<cut<end:end=cut
            src=np.ascontiguousarray(x[:,i:end])
            out=np.empty_like(src)
            s.stream_process(h,end-i,*src,*out,coef)
            y[:,i:end]=out;i=end;k+=1
        assert np.isfinite(y).all()
        return y,s.stream_worst(h)
    finally:s.stream_free(h)
def ideal(x):
    d=library();h=d.sj6_new(RATE);assert h
    try:
        d.sj6_params(h,DELAY/RATE,.62,.2,0)
        y=np.empty_like(x)
        d.sj6_process(h,x.shape[1],*x,*y)
        return y
    finally:d.sj6_free(h)
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--timing',action='store_true');ap.add_argument('--cutoff',type=float,default=3600);ap.add_argument('--taps',type=int,default=37);ap.add_argument('--stride',type=int,default=5);ap.add_argument('--delay',type=int,default=21876)
    a=ap.parse_args();global DELAY;DELAY=a.delay;here=Path(__file__).resolve().parent
    coef=np.rint(firwin(a.taps,a.cutoff,fs=RATE,window=('kaiser',5))*32768).astype('i2')
    scores=[]
    with tempfile.TemporaryDirectory() as tmp:
        s=load(here,tmp,a.taps,a.stride)
        assert s.stream_fir_taps()==len(coef) and s.stream_stride()==a.stride
        assert 32768*sum(abs(int(v)) for v in coef)<2**31
        for name in ('220','3000','5050','chord'):
            x=signal(name)
            y,w=render(s,x,coef)
            yp,wp=render(s,x,coef,True)
            np.testing.assert_array_equal(y.view('u4'),yp.view('u4'));assert w==wp
            original=ideal(x)
            if name!='chord':assert not np.any(y[1])
            for repeat in (1,2,3,4):
                slab=slice(DELAY*repeat-40,DELAY*repeat+3300)
                scores.append(dict(signal=name,repeat=repeat,**score(y[:,slab],original[:,slab])))
        x=signal('chord')
        y,w=render(s,x,coef,False,True)
        yp,wp=render(s,x,coef,True,True)
        np.testing.assert_array_equal(y.view('u4'),yp.view('u4'));assert w==wp<=58
        change=rms(y-render(s,x,coef)[0]);assert change>0
        result=dict(source_sha256=hashlib.sha256((here/'stream_probe.c').read_bytes()).hexdigest(),
                    state_bytes=s.stream_state(),ring_bytes=s.stream_ring(),host_coeff_bytes=s.stream_init_coeff_bytes(),
                    delay_frames=DELAY,stride=a.stride,fir_taps=a.taps,fir_cutoff_hz=a.cutoff,feedback=.62,variable_callback_sizes=True,
                    worst_decoded_frames_per_output_sample_after_time_edits=w,
                    edits_change_output=True,comparison=scores)
        if a.timing:
            ts=[]
            for i in range(5):
                start=time.thread_time_ns();render(s,x,coef,True,True)
                ts.append((time.thread_time_ns()-start)/1e6)
            result['host_python_and_c_only_ms_per_220000_frames']=round(statistics.median(ts),3)
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()
