"""Offline storage-only comparison across the 906 original programs."""
import hashlib,json,subprocess,sys,tempfile,os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import numpy as np
f32,h16,pack,summary=map(Path,sys.argv[1:5])
assert hashlib.sha256(pack.read_bytes()).hexdigest()=='d5e003e878ab46d6051d6cff16be557fbdfde7fccaa9255e4217bb5ffb51fa43'
env={**os.environ,'ROOM_TRACE':'/dev/null'}
for k in ('ROOM_RETURN','ROOM_FEED','WIDEN_TRACE'):env.pop(k,None)
def step(i):
    with tempfile.TemporaryDirectory(prefix="width-codec-bank-") as temp:
        out=Path(temp)
        reports=[]
        for exe,j in ((f32,'f32'),(h16,'h16')):
            r=subprocess.run([exe,pack,str(i),out/j,'wet'],env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,timeout=20)
            if r.returncode:
                return dict(i=i,error=r.returncode,description=r.stdout[-700:]+r.stderr[-600:])
            reports.append(r.stdout.splitlines()[0])
        a=np.fromfile(out/'f32','<f4').reshape(-1,2)
        b=np.fromfile(out/'h16','<f4').reshape(-1,2)
        assert a.shape==b.shape==(264600,2) and np.isfinite(a).all() and np.isfinite(b).all()
        side=a[:,1].astype('f8');q=b[:,1].astype('f8')
        mx=float(np.max(np.abs(side-q)))
        def residual(l,r):
            den=np.linalg.norm(side[l:r]); err=np.linalg.norm(side[l:r]-q[l:r])
            if not err:return None
            return float(20*np.log10(err/max(1e-20,den)))
        return dict(i=i,name=reports[0],left_exact=bool(np.array_equal(a[:,0].view('u4'),b[:,0].view('u4'))),
          exact=bool(np.array_equal(a.view('u4'),b.view('u4'))),right_peak_error=mx,
          right_raw_dbr=residual(0,264600),right_tail_raw_dbr=residual(220500,264600))
count=int(sys.argv[5]) if len(sys.argv)>5 else 906
with ThreadPoolExecutor(max_workers=5) as pool:
    data=[]
    for i,row in enumerate(pool.map(step,range(count)),1):
        data.append(row)
        if i%75==0:print(i,'/',count,flush=True)
results={'scope':'Same independent compact host baseline/control/room path, only source 50 ms right-history storage format; not authentic full-preset comparison or target timing.',
         'count':count,'left_exact_count':sum(x.get('left_exact',False) for x in data),
         'whole_exact_count':sum(x.get('exact',False) for x in data),'errors':[x for x in data if 'error'in x],'records':data}
summary.write_text(json.dumps(results,indent=2)+'\n')
print('complete',count,results['left_exact_count'],results['whole_exact_count'],len(results['errors']))
