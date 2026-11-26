from pathlib import Path
import sys
s=Path('sj6-ota-history/tanhreuse/auth.py').read_text().split('lib=C.CDLL(sys.argv[1])')[0]
s=s.replace("len(sys.argv)>4 and sys.argv[4]=='lower'","sys.argv[4]=='lower'")
s+='''co=np.array(coeff,dtype='f4')
libs=[C.CDLL(x) for x in sys.argv[1:4]]
for l in libs:l.physical_replay.argtypes=[C.c_void_p]*4+[C.c_uint]
pairs=[]
for i,k in enumerate((36,48,52,55)):
 l,r=by[k][:2];x=pre[i,l:r+1].copy()
 g,fb,h,blend,trim=extra[2:,i,l:r+1]
 v=np.stack((g,h,blend,trim))
 assert np.all(fb==0) and np.all(v==v[:,[0]])
 pp=v[:,0].copy(); host=[np.empty_like(x) for _ in libs]
 for solver,y in zip(libs,host):
    solver.physical_replay(x.ctypes.data,y.ctypes.data,pp.ctypes.data,co.ctypes.data,len(x))
 sides=[*host,extra[0,i,l:r+1]]
 pairs.append((k,l,r,sides))
def stats(y,ref):
 y= y.astype('f8');ref=ref.astype('f8');d=y-ref
 return dict(dbr=20*float(np.log10(np.linalg.norm(d)/np.linalg.norm(ref))),level_delta=20*float(np.log10(np.linalg.norm(y)/np.linalg.norm(ref))),peak_error=float(np.max(np.abs(d))))
rows={}
for start_name,a,b in [('attack',16384,20794),('hold',20794,38434),('release',38434,38993),('chord',53869,82534),('tail',82534,83093)]:
 for k,l,r,w in pairs:
   i,j=max(l,a),min(r+1,b)
   if j<=i:continue
   z=[x[i-l:j-l] for x in w]
   rows[f'{start_name}:{k}']={a:dict(ref=stats(z[i],z[0]),original=stats(z[i],z[3])) for i,a in enumerate(('base','fused','separate')) if i}
q=[np.concatenate([p[3][i] for p in pairs]) for i in range(4)]
print(json.dumps(dict(executable=binary,cutoff=controls[36],n=len(q[0]),raw_pcm_sha=sha(original.tobytes()),whole={a:dict(ref=stats(q[i],q[0]),original=stats(q[i],q[3])) for i,a in [(1,'fused'),(2,'separate')]},windows=rows),indent=2))
'''
Path(sys.argv[1]).write_text(s)
