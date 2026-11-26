from pathlib import Path
import sys
s=Path('sj6-ota-history/tanhreuse/auth.py').read_text().split('lib=C.CDLL(sys.argv[1])')[0]
s=s.replace("len(sys.argv)>4 and sys.argv[4]=='lower'","sys.argv[2]=='lower'")
s+='''co=np.array(coeff,dtype='f4')
base=Path(sys.argv[1]); pairs={}
for n in ('original','packed','separate'):
 ls=[C.CDLL(str(base/(n+t+'.so'))) for t in ('','-new')]
 for q in ls:q.physical_replay.argtypes=[C.c_void_p]*4+[C.c_uint]
 ref=[]; alt=[]
 for i,k in enumerate((36,48,52,55)):
  l,r=by[k][:2]; x=pre[i,l:r+1].copy()
  g,fb,h,blend,trim=extra[2:,i,l:r+1]; v=np.stack((g,h,blend,trim))
  assert np.all(fb==0) and np.all(v==v[:,[0]])
  pp=v[:,0].copy(); ys=[np.empty_like(x) for _ in ls]
  for q,y in zip(ls,ys):q.physical_replay(x.ctypes.data,y.ctypes.data,pp.ctypes.data,co.ctypes.data,len(x))
  assert ys[0].tobytes()==ys[1].tobytes()
  ref.append(ys[0]); alt.append(ys[1])
 pairs[n]=dict(words=sum(x.size for x in ref),before_sha=sha(np.concatenate(ref).tobytes()),after_sha=sha(np.concatenate(alt).tobytes()))
print(json.dumps(dict(executable=binary,cutoff=controls[36],raw_pcm_sha=sha(original.tobytes()),pairs=pairs),indent=2))
'''
Path(sys.argv[1]).write_text(s)
