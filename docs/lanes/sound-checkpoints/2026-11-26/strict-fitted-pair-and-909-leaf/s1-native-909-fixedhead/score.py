from pathlib import Path
import json,struct
def get(k):
 assert 'skipped 0 cycles' in Path(k+'.log').read_text()
 r=struct.unpack_from('<25I',Path(k+'.ram').read_bytes(),0x10000)
 assert r[0]==0x53413146
 return list(zip(r[1::3],r[2::3],r[3::3]))
aa,bb=get('base'),get('candidate')
res=[]
for label,a,b in zip(['kick 1.6','snare 2','tom 2','triangle corpus 2','both near zero','both near sign mirror','both cross mirror','exact phase corner'],aa,bb):
 assert a[1:]==b[1:]
 res.append(dict(mode=label,base_ticks=a[0],new_ticks=b[0],saved_pct=(a[0]-b[0])*100/a[0],phase=a[1],pcm_words_fnv32=a[2]))
print(json.dumps(dict(armed_code_and_table_size_B=[Path(k+'.bin').stat().st_size for k in ('base','candidate')],N_samples_per_case=8192,results=res),indent=2))
