import re,sys,struct
def stats(d):
 d=bytes(map(int,re.findall(r'\d+',open(d).read().split('{',1)[1])))
 step,n=struct.unpack_from('<HH',d)
 for k in range(n):
  p=4+2*n+struct.unpack_from('<H',d,4+2*k)[0]; width=d[p]&31; deg=d[p]>>5; q=d[p+5]; p+=6; s=p
  for j in range(deg+1):
   while d[p]&128: p+=1
   p+=1
  yield deg,p-s,q,width
base,new=sys.argv[1:]
for c in 'ae':
 a=list(stats(f'{base}/p_{c}.h')); b=list(stats(f'{new}/p_{c}.h'))
 d=[(x,y)for x,y in zip(a,b) if x!=y]
 print(c,'different',len(d),'higher degree',sum(y[0]>x[0] for x,y in d),'more coef bytes',sum(y[1]>x[1] for x,y in d),'q0 to q+',sum(x[2]==0<y[2] for x,y in d),'wider residual',sum(y[3]>x[3] for x,y in d))
