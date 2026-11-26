import hashlib,numpy as np
from pathlib import Path
p=Path('s1-fx-parity/reference-build/factory-inputs/public-morphs.f32')
assert hashlib.sha256(p.read_bytes()).hexdigest()=='0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'
v=np.fromfile(p,dtype='<f4').reshape(52,4097)[:,:4096].astype('f8')
idx=np.arange(4096)
shapes=1024,1024,4096,2048
table_count=sum(shapes)
for prec in (13,14,15):
 q=np.rint(v*(1<<prec)).astype('i8')
 total=np.zeros(5,int)
 worst=np.zeros(5)
 for b in range(1,13):
  size=[]
  er=[]
  for step in (1,2,4,8,16,32,64):
   z=0; e=0.; 
   for s,N in enumerate(shapes):
    row=q[b*4+s]
    y=row[::step]
    i=idx//step
    w=(idx%step)/step
    m=np.vstack([y[(i+j)%len(y)] for j in (-1,0,1,2)])
    # Catmull–Rom evaluated at exact full source indices.
    a,c,d,f=m
    x=(c+.5*w*(d-a+w*(2*a-5*c+4*d-f+w*(3*(c-d)-a+f))))
    if N==1024:
        k=idx&2047;mirror=k>1024
        j=np.where(mirror,2048-k,k)
        sign=np.where(idx>=2048,-1,1)*np.where(mirror & (s==0),-1,1)
    elif N==2048:
        j=np.where(idx>2048,4096-idx,idx)
        sign=np.where(idx>2048,-1,1)
    else:
        j=idx;sign=np.ones(4096)
    x=x[j]*sign
    e=max(e,abs(x/(1<<prec)-v[b*4+s]).max())
    z+=(N//step+(N!=4096))*2
   size.append(z);er.append(e)
  print(prec,b,list(zip((1,2,4,8,16,32,64),size,[round(x,7) for x in er])))
