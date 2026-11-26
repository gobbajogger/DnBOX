"""Independent shape experiment. No original waveform/coefficient inputs.
Does not yet attach to firmware; signed origin defaults are API parameters set
only from a source-proven convention. Band-specific harmonic cut is explicit.
"""
import math,cmath
from functools import lru_cache
T=math.tau
# Known simple mathematical asymmetry models (not a spectral clone).
def shaped_sine(t):
    q=T*t
    # Weak phase distortion analogue: both even and odd distortion, no lookup.
    return math.sin(q-(1/16)*math.sin(q)+(1/16)*math.sin(2*q))
def kink_triangle(t):
    t=t%1.; duty=.45
    return t*(2/duty)-1 if t<=duty else 1.-(t-duty)*2/(1.-duty)
@lru_cache(maxsize=None)
def terms(shape,cap):
    F=shaped_sine if shape==0 else kink_triangle
    N=2048
    x=[F(t/N) for t in range(N)]
    result=[sum(v*cmath.exp(-T*1j*h*i/N) for i,v in enumerate(x))/N for h in range(cap+1)]
    return tuple(result)
def synth(shape,N,H,rotate=0.,deemphasis=False):
    m=terms(shape,H)
    scale=1/(2*abs(m[1]))
    c=[0]+[m[h]*scale*((math.sin(math.pi*h/N)/(math.pi*h/N))**-2 if deemphasis else 1)
           for h in range(1,H+1)]
    return [2*sum((c[h]*cmath.exp(T*1j*h*(t/N+rotate))).real for h in range(1,H+1))
            for t in range(N)]
def variations():
    for shape in range(2):
      for n in (32,64,96):
       for h in (7,12,16):
        if h>=n/2:continue
        for q in (8,16):
            samples=synth(shape,n,h)
            maxval=max(abs(a) for a in samples)
            p=2**(q-1)-1
            si=[math.trunc(x*p/maxval)*maxval/p for x in samples]
            z=0.;e=0.
            for i in range(8192):
                zpos=n*i/8192
                a=int(zpos); v=(1-zpos+a)*si[a]+(zpos-a)*si[(a+1)%n]
                u=2*sum((c*cmath.exp(T*1j*j*i/8192)).real for j,c in enumerate(terms(shape,16)) if j)
                u/=2*abs(terms(shape,16)[1])
                e+=(v-u)**2;z+=u**2
            print(shape,n,h,q, round(10*math.log10(e/z),3), n*q//8*14)
if __name__=='__main__':variations()
