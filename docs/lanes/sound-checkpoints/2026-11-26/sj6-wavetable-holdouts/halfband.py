from cmath import phase,exp
from math import pi,tau
from struct import unpack,pack
aw=(.079866424202919,.5453536510467529)
ae=(.28382933139801025,.8344119191169739)
def H(w):
    def sec(a):
        z=exp(-1j*w)
        return (a+z)/(1+a*z)
    return .5*(sec(ae[0])*sec(ae[1])+exp(1j*.5*w)*sec(aw[0])*sec(aw[1]))
for note in (45,57,69,81,93,105):
    x=440*2**((note-69)/12)/44100
    w=tau*x
    h=H(w)*H(w/2)
    print(note,'analytic H angle',phase(h)*180/pi,'dc slope',-360*x*.97079264196,'delta',phase(h)*180/pi+360*x*.97079264196)
from math import log10
k=[(1-a)/(1+a) for a in (*aw,*ae)]
D=1.5*(.5*sum(k)-.25)
C=.5*sum(x-x**3 for x in k)/12*(1+1/8)*(tau*tau)
print('derived D,C',D,C,D.hex(),C.hex())
for n in (45,57,69,81,93,105):
 x=440*2**((n-69)/12)/44100
 exact=phase(H(tau*x)*H(tau*x/2))*180/pi
 pred=-360*x*(D+C*x*x)
 print(n, 'remaining analytical cubic angle error', pred-exact)
