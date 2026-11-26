"""Two rounded A4 validation anchors per shape, one global law, no raw table.
H2/H3 (rounded dBc and phase) are the only measured fitted quantities.
"""
import math,cmath
# rounded, readable values; compiled native table regenerated from independent model.
ANCHORS=(((-30.67,-105.84),(-29.87,176.49)),
         ((-22.33,147.89),(-19.14,-6.54)))
def synth_sparse(shape,N,H,compensate=True):
    c=[0,1.]
    for h in range(2,H+1):
        index=(h&1)
        a,deg=ANCHORS[shape][index]
        ref=2+index
        if shape==0 and h>3:
            c.append(0.)
            continue
        if shape==1:
            # One independent triangular rolloff applied globally.
            a+=40*math.log10(ref/h)
            deg=deg*h/ref if index else deg-9*(h-ref)
        factor=(math.sin(math.pi*h/N)/(math.pi*h/N))**-2 if compensate else 1
        c.append(10**(a/20)*factor*cmath.exp(1j*math.radians(deg)))
    if compensate:c[1]=(math.sin(math.pi/N)/(math.pi/N))**-2
    return [sum((c[h]*cmath.exp(2j*math.pi*h*k/N)).real for h in range(1,H+1)) for k in range(N)]
