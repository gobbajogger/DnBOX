"""Compare the C model to the privately supplied ARM64 kernel, no AU host."""
import argparse
import ctypes
import json
import numpy as np
from arm_delay_oracle import CPU, Original
from sj6_stereo_delay import library

def main():
    p=argparse.ArgumentParser();p.add_argument('bin');args=p.parse_args()
    cpu=CPU(args.bin); d=library(); results=[]
    rng=np.random.default_rng(303909)
    for rate in (44100,48000):
        for ping in (False,True):
            for kind in ('pulse','stereo_noise','retime','clamp'):
                n=96
                x=np.zeros((2,n),np.float32)
                if kind in ('stereo_noise','clamp'): x[:]=rng.uniform(-.75,.75,x.shape)
                else:x[0,0]=.73;x[1,39]=.43
                sec=(4.0 if kind=='pulse' else 17.37)/rate
                dry=-.25 if kind=='clamp' else .27
                fb=1.1 if kind=='clamp' else .64
                original=Original(cpu,rate)
                model=d.sj6_new(rate)
                ys=[]; refs=[]; pos=0
                try:
                    for count in (7,16,25,48):
                        if pos==23 and kind=='retime':sec=7.7/rate
                        original.set(sec,fb,dry,ping)
                        d.sj6_params(model,sec,fb,dry,ping)
                        in_=np.ascontiguousarray(x[:,pos:pos+count]);out=np.empty_like(in_)
                        refs.append(np.array(original.process(*in_),dtype=np.float32))
                        d.sj6_process(model,count,*in_,*out)
                        ys.append(out)
                        pos+=count
                finally: d.sj6_free(model)
                y=np.concatenate(ys,axis=-1); ref=np.concatenate(refs,axis=-1)
                np.testing.assert_array_equal(y.view('u4'),ref.view('u4'))
                results.append(dict(rate=rate,ping=ping,case=kind,pcm_words=y.size,bit_equal=True))
    print(json.dumps(dict(build9_core=results,words=sum(i['pcm_words'] for i in results)),indent=2))
if __name__=='__main__': main()
