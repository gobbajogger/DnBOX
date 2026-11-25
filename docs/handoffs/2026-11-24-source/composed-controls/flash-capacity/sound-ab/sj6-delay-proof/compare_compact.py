"""Isolated, settled non-ping-pong comparison against the checked host core."""
import argparse,ctypes as C,hashlib,json,math,os,subprocess,tempfile
from pathlib import Path
import numpy as np
from sj6_stereo_delay import library

def main():
    ap=argparse.ArgumentParser();ap.add_argument('source',type=Path)
    a=ap.parse_args(); rate=44100; n=60000
    compact_dir=a.source.resolve()
    source=compact_dir/'juno6_delay_native.inc'
    desc=[];ref=library()
    fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
    with tempfile.TemporaryDirectory() as tmp:
        so=Path(tmp)/'compact.so'
        subprocess.run([os.environ.get('CC','cc'),'-std=c11','-O2','-fPIC','-shared','-ffp-contract=off','-I',str(compact_dir),str(Path(__file__).with_name('compact_probe.c')),'-lm','-o',str(so)],check=True)
        d=C.CDLL(str(so));d.compact.argtypes=[C.c_uint]*3+[fp,fp,C.c_uint]
        d.compact.restype=C.c_uint
        for raw in (2,42,85):
            for hz in (220,3000):
                x=np.zeros((2,n),np.float32)
                t=np.arange(2400)/rate
                x[0,:len(t)]=.4*np.sin(2*np.pi*hz*t)*np.hanning(len(t))
                compact=x.copy(); orig=np.empty_like(x)
                sec=np.float32(np.float32(1.5/127)*raw)
                dry=np.float32(1-.9)
                # Test the first echo, with no feedback: exposes only the
                # history/interpolator, not the level or recirculation laws.
                state=ref.sj6_new(rate)
                try:
                    ref.sj6_params(state,sec,0,dry,0)
                    ref.sj6_process(state,n,*x,*orig)
                finally:ref.sj6_free(state)
                stride=d.compact(raw,0,127,*compact,n)
                start=math.floor(rate*sec);end=start+len(t)+stride+8
                o=(orig[0]-x[0]*dry)[start:end]
                c=(compact[0]-x[0]*dry)[start:end]
                rms=lambda z: float(np.sqrt(np.mean(z.astype(np.float64)**2)))
                desc.append(dict(seconds=float(sec),hz=hz,stride=stride,
                    effective_rate_hz=rate/stride,level_error_db=round(20*math.log10(rms(c)/rms(o)),3),
                    normalized_residual_db=round(20*math.log10(rms(c-o)/rms(o)),3)))
    print(json.dumps(dict(compact_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),cases=desc),indent=2))
if __name__=='__main__':main()
