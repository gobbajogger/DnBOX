import math,struct,zlib
import numpy as np
def zip_bytes(x):
 a=zlib.compressobj(6,wbits=-15);return a.compress(x)+a.flush()
def fold(a):
 # Alternative within-history lanes: both long-line and both short-line sides.
 # XOR is reversible over actual IEEE words, not subtract-to-mono.
 b=a.copy()
 if b.shape[1]>1:b[:,1::2]^=b[:,::2]
 return b
def pred(a,i):
 if i==0:return a
 b=a.copy()
 if i==1:b[1:]=a[1:]^a[:-1]
 if i==2:b[1:]=a[1:]-a[:-1]
 if i==3:
  if len(b)>1:b[1]=a[1]-a[0]-a[0]
  if len(b)>2:b[2:]=a[2:]-2*a[1:-1]+a[:-2]
 return b
def undo(a,i):
 if not i:return a
 if i==1:return np.bitwise_xor.accumulate(a,axis=0,dtype=a.dtype)
 if i==2:return np.add.accumulate(a,axis=0,dtype=a.dtype)
 return np.add.accumulate(np.add.accumulate(a,axis=0,dtype=a.dtype),axis=0,dtype=a.dtype)
def shuffle(a):
 n=a.dtype.itemsize;m=a.shape[1]
 return a.T.copy().view('u1').reshape(m,len(a),n).transpose(0,2,1).tobytes()
def unshuffle(b,dims,dt):
 row,width=dims;n=dt.itemsize
 return np.frombuffer(b,'u1').reshape(width,n,row).transpose(0,2,1).copy().reshape(width,row*n).view(dt).T
def rice(b,k):
 # Implementation-faithful unary+remainder packing; residual words reduced
 # to signed ZigZag in modulo integer arithmetic. Seeds carried explicitly.
 q=b>>k; count=int(q.astype('u8').sum())+(k+1)*b.size
 if count>8*b.nbytes:return None
 bits=bytearray((count+7)//8); i=0
 for w in b.ravel():
  w=int(w);i+=w>>k;bits[i>>3]|=1<<(i&7);i+=1
  for j in range(k):
   bits[i>>3]|=((w>>j)&1)<<(i&7);i+=1
 return bytes(bits)
def unrice(b,k,shape,dt):
 out=np.empty(shape,dtype=dt);flat=out.ravel();i=0
 for j in range(flat.size):
  n=0
  while not (b[i>>3]>>(i&7)&1):n+=1;i+=1
  i+=1; r=0
  for v in range(k):r|=(b[i>>3]>>(i&7)&1)<<v;i+=1
  flat[j]=(n<<k)|r
 return out
def entropy_est(u):
 bits=u.dtype.itemsize*8
 f=u.astype(np.uint64)
 return min([((k+1)*u.size+int((f>>k).sum())+7)//8 for k in range(bits-1)])
def encode(a,allow_rice=True):
 assert a.flags.c_contiguous and a.dtype.kind=='u'
 best=(0,a.tobytes(),0,0,0)
 # Split raw vs exact modulo word predictors. All dictionary state starts new.
 for cross in range(2 if a.shape[1]>1 else 1):
  A=fold(a) if cross else a
  for p in range(4):
   x=pred(A,p)
   b=zip_bytes(shuffle(x))
   if len(b)<len(best[1]):best=(1,b,cross,p,0)
   if allow_rice and p in (2,3):
    cut=p-1
    s=x[cut:];w=a.dtype.itemsize*8
    z=(s<<1)^((np.zeros_like(s)-((s>>(w-1)))))
    f=z.astype('u8'); opts=[((k+1)*z.size+int((f>>k).sum()),k) for k in range(w-1)]
    estimate,k=min(opts)
    if (estimate+7)//8+cut*a.shape[1]*a.dtype.itemsize+1<len(best[1]):
     comp=rice(z,k)
     if comp is not None:best=(2,x[:cut].tobytes()+comp,cross,p,k)
 mode,b,c,p,k=best
 # 1-byte kind/mode, one codec param, 16-bit payload length, then payload
 assert len(b)<65536
 return struct.pack('<BBH',mode|(c<<2)|(p<<3),k,len(b))+b
def decode(b,sh,dt):
 flag,k,n=struct.unpack_from('<BBH',b); assert len(b)==4+n
 mode=flag&3;c=(flag>>2)&1;p=(flag>>3)&3
 if not mode:return np.frombuffer(b[4:],dt).reshape(sh)
 if mode==1:
  a=unshuffle(zlib.decompress(b[4:],wbits=-15),sh,dt)
 else:
  cut=p-1;n=dt.itemsize*sh[1]*cut
  a=np.empty(sh,dtype=dt);a[:cut]=np.frombuffer(b[4:4+n],dt).reshape(cut,sh[1])
  v=unrice(b[4+n:],k,(sh[0]-cut,sh[1]),dt)
  a[cut:]=(v>>1)^(np.zeros_like(v)-(v&1))
 q=undo(a,p)
 return fold(q) if c else q
