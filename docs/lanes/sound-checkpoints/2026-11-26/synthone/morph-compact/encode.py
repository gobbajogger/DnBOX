"""Static, bounded random access. Optimizes per table, not per program."""
import argparse,hashlib,json
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser()
p.add_argument('waves',type=Path); p.add_argument('output',type=Path);p.add_argument('--tol',type=float,required=True)
p.add_argument('--share',action='store_true')
a=p.parse_args()
H='0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'
assert hashlib.sha256(a.waves.read_bytes()).hexdigest()==H
v=np.fromfile(a.waves,dtype='<f4').reshape(52,4097)[:,:4096].astype('f8')
q=np.rint(v*16384).astype('i8')
idx=np.arange(4096); ns=(1025,1025,4096,2049)
from struct import pack
def residual(x,bs):
 index=bytearray(); tail=bytearray()
 for st in range(0,len(x),bs):
    w=x[st:st+bs]; n=len(w); k=np.arange(n)
    slope=round((int(w[-1])-int(w[0]))/max(1,n-1))
    err=w-w[0]-slope*k
    zz=np.where(err<0,-2*err-1,2*err)
    bw=int(zz.max()).bit_length()
    at=len(index)+len(tail)
    index+=pack('<H',len(tail))
    if -(1<<15)<=slope<1<<15 and 5+(bw*n+7)//8<1+2*n and bw<16:
        tail+=pack('<Bhh',bw,int(w[0]),slope)
        s=sum(int(y)<<(j*bw) for j,y in enumerate(zz))
        tail+=s.to_bytes((bw*n+7)//8,'little')
    else:
        tail+=pack('<B',16)+w.astype('<i2').tobytes()
 return bytes(index+tail),len(index)
def short(row,shape,step,kernel):
 N=ns[shape]; y=row[::step]
 k=idx if N==4096 else np.arange(N)
 i=k//step; f=(k%step)/step
 if kernel==1:
    a,b=[y[(i+j)%len(y)].astype('f4') for j in (0,1)]
    z=a+f*(b-a)
    start,end=0,int(i[-1])+1
 else:
    a,c,d,e=[y[(i+j)%len(y)].astype('f4') for j in (-1,0,1,2)]
    z=c+.5*f*(d-a+f*(2*a-5*c+4*d-e+f*(3*(c-d)-a+e)))
    start,end=-1,int(i[-1])+2
 y=y[np.arange(start,end+1)%len(y)]
 return y.astype('<i2').tobytes(),z/16384,0
def expand(half,s):
 if s<2:
    k=idx&2047; mirror=k>1024
    j=np.where(mirror,2048-k,k)
    sign=np.where(idx>=2048,-1,1)*np.where(mirror&(s==0),-1,1)
 elif s==3:
    j=np.where(idx>2048,4096-idx,idx); sign=np.where(idx>2048,-1,1)
 else:return half
 return half[j]*sign
analytic=np.stack([(np.minimum(idx,4096-idx)/1024)-1,np.where(idx<2048,-1,1),np.where(idx<512,-1,1),idx/2048-1])
assert np.array_equal(v[:4],analytic)
payload=bytearray(); rows=[]; audit=[]
if a.share:
 for r in range(4,52,4):
    saw=v[r+3]
    assert np.max(abs(v[r+1]-(40/39)*(saw-np.roll(saw,2048))))<2e-7
    assert np.max(abs(v[r+2]-(.75+(3.2/np.pi)*(saw-np.roll(saw,512)))))<2e-7
for row in range(4,52):
 shape=row%4; n=ns[shape]
 if a.share and shape in (1,2):
    rows.append((0,0,4,0))
    audit.append({'row':row,'bytes':0,'mode':4,'shift':0,'error':None})
    continue
 ref=v[row]
 err=lambda z:float(np.max(abs(expand(z,shape)-ref)))
 options=[]
 for bs in (16,32,64,128):
    blob,idxbytes=residual(q[row,:n],bs)
    options.append((blob,idxbytes,0,bs.bit_length()-1,err(q[row,:n]/16384)))
 for j in range(1,8):
    for kern in (1,3):
        blob,z,k=short(q[row],shape,1<<j,kern)
        if err(z)<=a.tol: options.append((blob,k,kern,j,err(z)))
 blob,idxbytes,mode,shift,e=min(options,key=lambda x:len(x[0]))
 assert e<=a.tol and len(payload)<65536
 rows.append((len(payload),idxbytes,mode,shift))
 audit.append({'row':row,'bytes':len(blob),'mode':mode,'shift':shift,'error':e})
 payload.extend(blob)
payload+=b'\0\0'  # decoder's fixed-width residual fetch may span last used byte
assert len(payload)<65536
a.output.mkdir(exist_ok=True,parents=True)
h=['#define DNBX_MORPH_SAW_SHARE '+str(int(a.share)),'static const struct {uint16_t at, index_bytes; uint8_t mode, shift;} morph_rows[48]={']
h.extend('{%d,%d,%d,%d},'%r for r in rows)
h+=['};','static const uint8_t morph_bytes[%d]={'%len(payload)]
for i in range(0,len(payload),24): h.append(','.join(map(str,payload[i:i+24]))+',')
h+=['};']
(a.output/'morph-data.h').write_text('\n'.join(h)+'\n')
(a.output/'format.json').write_text(json.dumps(dict(input_sha256=H, payload_bytes=len(payload), row_directory=6*len(rows), bytes=len(payload)+6*len(rows), tolerance=a.tol*(2*40/39 if a.share else 1)+4e-7, primitive_tolerance=a.tol,shared=a.share,rows=audit),indent=2)+'\n')
