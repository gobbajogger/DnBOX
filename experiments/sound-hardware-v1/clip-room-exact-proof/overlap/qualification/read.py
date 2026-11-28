from pathlib import Path
from struct import unpack_from
import json, subprocess, sys
s = {}
for l in subprocess.check_output(['readelf','-Ws',sys.argv[1]],text=True).splitlines():
    f=l.split()
    if len(f)>=8 and f[3]=='OBJECT':s[f[7]]=int(f[1],16)-0x1c00000
b=Path(sys.argv[2]).read_bytes()
r=lambda k,n:unpack_from('<'+str(n)+'I',b,s[k])
cols=['n','909_complete','808_complete','B_start','B_end','claim_retries','first_work_frame','tiles','worker_frames','worker_ticks_us','spin_us','join_wait_us','fallbacks','phase_join','planner_mask']
res={}
for k in ('worst','last'):
    flat=r('lazy_'+k,64)
    rows=[flat[i:i+16] for i in range(0,len(flat),16)]
    res[k]={'span':[],'us':r('lazy_'+k+'_us',1)[0]}
    for a in [x for x in rows if x[0]]:
        n,t,kit9,kit8,bs,be,retry,first,tile,cov,ww,spin,jw,fb,end,mask=a
        d=lambda v:round(((v-t)&0xffffffff)/24,2) if v else None
        row=[n,d(kit9),d(kit8),d(bs),d(be),retry,first-1 if first else None,tile,cov,round(ww/24,2),round(spin/24,2),round(jw/24,2),fb,d(end),mask]
        res[k]['span'].append(dict(zip(cols,row)))
res['worst_idx']=r('lazy_worst_half',1)[0]
res['audio_xruns']=r('audio_xruns',1)[0]
print(json.dumps(res,indent=2))
