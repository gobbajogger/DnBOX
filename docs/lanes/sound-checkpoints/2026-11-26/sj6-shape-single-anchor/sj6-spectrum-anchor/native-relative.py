"""Compare generated private receiver at original note time to fixed AU metadata.
Only relative harmonic shape is compared; no output gain or alignment match."""
import numpy as np
import argparse, ctypes as c, json
def main():
    p=argparse.ArgumentParser()
    p.add_argument('model');p.add_argument('native_summary')
    a=p.parse_args()
    f=c.c_float
    so=c.CDLL(a.model).sj6_anchor_run
    so.argtypes=[c.POINTER(f),c.c_uint,f,c.POINTER(f),c.c_uint]
    x=json.load(open(a.native_summary))
    r=[]
    for u in x:
        note=u['key']; sh=u['shape']
        freq=440*2**((note-69)/12)
        state=(f*17)(); n=55000-16384; out=(f*n)()
        so(state,sh=='tri',freq/44100,out,n)
        # Same projection basis and exact window as authenticated scorer.
        y=np.ctypeslib.as_array(out)[25000-16384:].astype('f8')
        d=np.array(u['before_scoped_transfer_relative_db'])[:,0]
        deg=np.array(u['before_scoped_transfer_relative_phase_degrees'])[:,0]
        h=10**(d/20)*np.exp(1j*np.deg2rad(deg))
        order=np.arange(1,len(d)+1)
        ang=2*np.pi*freq/44100*np.outer(np.arange(25000,55000)-16384,order)
        G=np.column_stack((np.cos(ang),np.sin(ang),np.ones(len(y))))
        q=np.linalg.lstsq(G,y,rcond=None)[0]
        cm=q[:len(d)]-1j*q[len(d):2*len(d)]
        rel=cm/abs(cm[0])*np.exp(-1j*order*np.angle(cm[0]))
        residual=np.linalg.norm(rel[1:]-h[1:])/np.linalg.norm(h)
        r.append(dict(note=note,shape=sh,repeat=u['repeat'],scope='anchor fit' if note==69 else 'blind holdout',
                      harmonic_relative_residual_dbr=float(20*np.log10(residual)),
                      candidate_pretap_h1_peak=float(abs(cm[0])),candidate_pretap_h1_phase_deg=float(np.angle(cm[0],deg=True)),
                      source_pcm_sha256=u['pcm_sha256']))
    print(json.dumps(r,indent=2))
if __name__=='__main__':main()
