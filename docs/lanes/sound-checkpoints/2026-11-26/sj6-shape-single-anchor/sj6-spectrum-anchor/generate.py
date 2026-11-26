"""One calibration anchor per shape; never load holdouts in this program.
Inputs: JSON shape/harmonic ratios referred to h1 and exact transfer used at A4.
Dataset and generated header are intentionally not part of source-only archives.
"""
import argparse, cmath, json, math
def halfband(w):
    z=cmath.exp(-1j*w)
    def section(a): return (a+z)/(1+a*z)
    return .5*(section(.28382933139801025)*section(.8344119191169739)+cmath.exp(.5j*w)*section(.079866424202919)*section(.5453536510467529))
def biquad(w,c):
    z=cmath.exp(-1j*w)
    b0,b1,b2,a1,a2=c
    return (b0+b1*z+b2*z*z)/(1+a1*z+a2*z*z)
def dc(w,p):
    z=cmath.exp(-1j*w)
    return (1-z)/(1-p*z)
om=math.tau*10/44100
cos,sin=math.cos(om),math.sin(om)
alpha=sin/2
hp=tuple(v/(1+alpha) for v in ((1+cos)/2,-(1+cos),(1+cos)/2,-2*cos,1-alpha))
def suffix(w,c):
    return biquad(w,c)*biquad(w,hp)*dc(w,float.fromhex('0x1.fd70a4p-1'))*dc(w,float.fromhex('0x1.ff7ceep-1'))
def transfer(w,c,native):
    h=halfband(w)*halfband(w*.5)
    return h*(suffix(w,c) if native else biquad(w,c))
def generate(packet,N,Q,H):
    assert packet['key']==69 and packet['sample_rate']==44100
    assert N>2*H and Q in [8,16] and H<=18
    maxq=(1<<(Q-1))-1
    for shape in ('sine','tri'):
        p=packet[shape]
        c=p['normal_df1']
        w=math.tau*440/44100
        base=transfer(w,c,packet.get("native_suffix",False))
        # Inputs use signed cos projection, preserve absolute starting phase
        # from the actual physical phase-zero owner, not measured time shift.
        terms=[1+0j]
        for n,d in enumerate(p['harmonics_h2_to_h18'][:H-1],start=2):
            db,deg=d
            terms.append(10**(db/20)*cmath.exp(1j*math.radians(deg))*
                         abs(base)*cmath.exp(1j*n*cmath.phase(base))/transfer(n*w,c,packet.get("native_suffix",False)))
        caps=(14,14,14,8,4,2,1) if shape=='sine' else (46,32,16,8,4,2,1)
        rows=[[sum((h*cmath.exp(1j*math.tau*j*(k+1)/N)).real for k,h in enumerate(terms[:cap]))
               for j in range(N)] for cap in caps]
        peak=max(abs(v) for r in rows for v in r)
        yield peak/maxq,[tuple(round(v*maxq/peak) for v in r) for r in rows]
if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('anchor');p.add_argument('--size',type=int,default=128);p.add_argument('--width',type=int,default=16);p.add_argument('--harmonics',type=int,default=18)
    a=p.parse_args()
    out=list(generate(json.load(open(a.anchor)),a.size,a.width,a.harmonics))
    print('#include <stdint.h>')
    print(f'enum {{ TBL_N={a.size}, TBL_Q={a.width}, TBL_H={a.harmonics} }};')
    print(f'static const int{a.width}_t TBL[2][7][{a.size}]={{')
    for _,rows in out:
        print(' {')
        for r in rows: print('  {'+','.join(map(str,r))+'},')
        print(' },')
    print('};')
    print('static const float TBL_GAIN[2]={'+','.join(float(amp).hex()+'f' for amp,_ in out)+'};')
