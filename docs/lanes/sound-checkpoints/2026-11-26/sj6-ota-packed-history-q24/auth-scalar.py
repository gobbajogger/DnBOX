"""Source-translated Q24 witness; not execution of candidate firmware C."""
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
import sys
import numpy as np

F=lambda x:C.c_float(x).value
get=struct.Struct('<f').pack
f=struct.Struct('<f').unpack
u=struct.Struct('<I').unpack
put=struct.Struct('<I').pack
tl=C.CDLL('libm.so.6').tanhf
tl.argtypes=[C.c_float]
tl.restype=C.c_float

def rne(x):
 b=u(get(x))[0]
 return f(put((b+0x7f+((b>>8)&1))&0xffffff00))[0]

def render(x,v,q,m):
 g,high,blend,trim=map(float,v)
 q=list(map(float,q))
 gz=F(g/F(1+g));zi=F(1-gz)
 ap=[0.]*8;pole=[0.]*4;cond=0.;out=np.zeros(len(x),np.float32)
 def run_ap(x,j,i):
  mid=F(F(q[i]*x)+ap[j]);z0=F(x-F(q[i]*mid))
  y=F(F(q[i+1]*mid)+ap[j+1]);z1=F(mid-F(q[i+1]*y))
  if j<m:z0=rne(z0);z1=rne(z1)
  ap[j]=z0;ap[j+1]=z1
  return y
 def nonlinear(a):
  nonlocal cond
  dv=F(F(a-cond)*high);lp=F(cond+dv);cond=F(dv+lp)
  z=passed=F(F(a-lp)*q[4]);est=[0.]*4
  for i in range(4):
   z=F(F(gz*z)+F(zi*pole[i]));est[i]=z
  z=passed
  for i in range(4):
   p=est[i];last=pole[i];n=tl(F(z-p))
   derivative=F(g*F(1-F(n*n)));inv=F(1/F(1+derivative))
   residual=F(-F(F(-g*n)+F(p-last))*inv)
   y=F(p+residual);pole[i]=F(F(2*y)-last);z=p;est[i]=y
  a,b=est[1],F(est[3]*trim)
  return b if blend==1 else a if blend==0 else F(F(blend*F(b-a))+a)
 for i,v in enumerate(x):
  a=run_ap(float(v),0,0);b=run_ap(float(v),2,2)
  a=nonlinear(a);b=nonlinear(b)
  b=run_ap(b,4,0);a=run_ap(a,6,2);out[i]=F(40*F(F(a+b)*.5))
 return out

def signed(a,b):
 a,b=a.astype('f8'),b.astype('f8')
 return float(20*np.log10(np.linalg.norm(a-b)/np.linalg.norm(b)))

def main():
 s=Path('sj6-ota-history/auth-compare.py').read_text().split('lib=C.CDLL(sys.argv[1])')[0]
 rows={}
 for alt in ('authored','lower'):
  sys.argv=['w','x','x','x',alt]
  d={};exec(compile(s,'tap','exec'),d)
  pre,extra,by,coeff=(d[n] for n in ('pre','extra','by','coeff'))
  assert hashlib.sha256(pre.tobytes()).hexdigest()=='7ab7581651b9de789ca29dfb3b2da12741f81adf182bee745eab80f6a872490a'
  r={}
  for i,k in enumerate((36,48,52,55)):
   l,h=by[k][:2]
   x=pre[i,l:h+1];original=extra[0,i,l:h+1]
   g,rz,hi,b,t=extra[2:,i,l:h+1]
   assert np.all(rz==0) and all(np.all(a==a[0]) for a in (g,hi,b,t))
   n=len(x)
   x=np.concatenate((x,np.zeros(4096,np.float32)))
   y,a4,a8=(render(x,[g[0],hi[0],b[0],t[0]],coeff,m) for m in (0,4,8))
   r[k]=[signed(y[:n],original),signed(a4[:n],y[:n]),signed(a4[:n],original),signed(a4[n:],y[n:]),
         signed(a8[:n],y[:n]),signed(a8[:n],original),signed(a8[n:],y[n:])]
  rows[alt]=r
 print(json.dumps(rows,indent=2,allow_nan=False))
if __name__=='__main__':
 main()
