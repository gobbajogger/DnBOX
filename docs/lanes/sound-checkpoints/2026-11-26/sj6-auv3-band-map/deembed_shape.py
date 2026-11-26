"""Remove only previously identified linear transfer from measured descriptors."""
import ctypes,json,sys,numpy as np
p,lib=sys.argv[1:3]
d=ctypes.CDLL(lib);d.coeff.argtypes=[ctypes.c_double,ctypes.c_double,ctypes.POINTER(ctypes.c_double)]
a=np.zeros(5);d.coeff(44100.,20000.,a.ctypes.data_as(ctypes.POINTER(ctypes.c_double)))
flt=lambda w:float(np.float32(float.fromhex(w)))
b=flt('0x1.ffa2ap-1'); k=flt('0x1.10427p-19'); q=flt('0x1.ff4564p-1')
cr=flt('0x1.fd70a4p-1'); dc=float(np.float32(.999))
out=[]
for r in json.load(open(p)):
 m=np.array(r['harmonics_relative_db']); deg=np.array(r['relative_phases_degrees'])
 n=np.arange(1,len(m)+1)[:,None]; f=r['fundamental_hz']
 z=np.exp(-2j*np.pi*n*f/44100)
 Hnorm=(a[0]+a[1]*z+a[2]*z*z)/(1+a[3]*z+a[4]*z*z)
 hp=b*(1-z)**2/((1-z)*(1-q*z)+k*z)
 dcr=(1-z)/(1-cr*z); dcf=(1-z)/(1-dc*z)
 s=Hnorm*hp*dcr*dcf
 relative_transfer_db=20*np.log10(abs(s)/abs(s[0]))
 relative_transfer_phase=np.angle(s*np.exp(-1j*n*np.angle(s[:1])),deg=True)
 phases=((deg-relative_transfer_phase+180)%360)-180
 row={'shape':r['shape'],'key':r['key'],'repeat':r['repeat'],'pcm_sha256':r['pcm_sha256'],
      'before_scoped_transfer_relative_db':(m-relative_transfer_db).tolist(),'before_scoped_transfer_relative_phase_degrees':phases.tolist()}
 out.append(row)
print(json.dumps(out,indent=2))
